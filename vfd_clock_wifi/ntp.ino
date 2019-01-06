
/*
  NTP query
*/


#include <Time.h>
#include <TimeLib.h>


#define NTP_UDP_PACKET_SIZE       48
#define NTP_UDP_RSP_TIMEOUT_MS    5000


byte                  ntp_exec_state    = 0;
DNS_HOST_BY_NAME_IO   ntp_dns_io;
char                  ntp_hostname[NTP_HOSTNAME_MAX_LEN+1];     // NTP hostname, zero-terminated string
WiFiUDP               ntp_udp;
byte                  ntp_udp_tx_packet[NTP_UDP_PACKET_SIZE];   // Buffer for outgoing packets
byte                  ntp_udp_rx_packet[NTP_UDP_PACKET_SIZE];   // Buffer for incoming packets
uint32_t              ntp_udp_start_ms;
NTP_REPORT_TIME_FN   *ntp_report_time_fn;


void  NTP_Exec_Set_State (byte s)
{
  byte  prev_s;

  prev_s = ntp_exec_state;

  ntp_exec_state = s;

#ifdef  NTP_DEBUG
  //Serial.printf("ntp_exec_state %u->%u\n",prev_s,s);
#endif
}


void  NTP_Exec ()
{
  switch (ntp_exec_state)
  {
    case 0:
    {
      // Idle
      return;
    }

    case 1:
    {
      if (settings.ntp_use_addr)
      {
        // Apply IP address
        ntp_dns_io.res_ad = settings.ntp_addr;

        NTP_Exec_Set_State(3);
        return;
      }
      else
      {
        DNS_Host_By_Name_Init(&ntp_dns_io);
  
        // Wait for completion of the DNS lookup
        NTP_Exec_Set_State(2);
        return;
      }
    }

    case 2:
    {
      if (DNS_Host_By_Name_Has_Completed(&ntp_dns_io))
      {
        if ((uint32_t)0 == ntp_dns_io.res_ad)
        {
          // DNS lookup failed

          NTP_Exec_Set_State(0);
          return;
        }
        else
        {
          NTP_Exec_Set_State(3);
          return;
        }
      }

      return;
    }

    case 3:
    {
      // Send packet for requesting timestamp. NTP requests are to port 123.
      ntp_udp.beginPacket(ntp_dns_io.res_ad,123);
      ntp_udp.write(ntp_udp_tx_packet,NTP_UDP_PACKET_SIZE);
      ntp_udp.endPacket();

      // Start timer
      ntp_udp_start_ms = millis();

      // Wait for response from NTP server
      NTP_Exec_Set_State(4);
      return;
    }

    case 4:
    {
      int   cnt;

      cnt = ntp_udp.parsePacket();
      if (cnt > 0)
      { 
#ifdef  NTP_DEBUG
        Serial.printf("NTP query: packet received %d bytes\n",cnt);
#endif

        if (cnt >= NTP_UDP_PACKET_SIZE)
        {
          // Read server response packet into buffer
          ntp_udp.read(ntp_udp_rx_packet,NTP_UDP_PACKET_SIZE);

          // Process response

          time_t hi32            = word(ntp_udp_rx_packet[40],ntp_udp_rx_packet[41]);
          time_t lo32            = word(ntp_udp_rx_packet[42],ntp_udp_rx_packet[43]);
          time_t secs_since_1900 = hi32 << 16 | lo32;

          // Unix time starts on Jan 1 1970. Subtract Unix epoch from seconds since 1900.
          time_t t = secs_since_1900 - 2208988800UL;

#ifdef  NTP_DEBUG
          Serial.printf("* time_t: %u\n",t);
          Serial.printf("* UTC:    %04d-%02d-%02d %02d:%02d:%02d\n",
                        year(t),month(t),day(t),
                        hour(t),minute(t),second(t));
#endif

          if (ntp_report_time_fn) ntp_report_time_fn(t);
        }
        else
        {
          // Response packet is too small
        }

        NTP_Exec_Set_State(0);
        return;
      }
      else
      {
        if ((millis() - ntp_udp_start_ms) > NTP_UDP_RSP_TIMEOUT_MS)
        {
#ifdef  NTP_DEBUG
          Serial.println("NTP query: timeout");
#endif

          NTP_Exec_Set_State(0);
          return;
        }
      }

      return;
    }
  }
}


boolean  NTP_Is_Busy ()
{
  return (ntp_exec_state != 0);
}


void  NTP_Start (NTP_REPORT_TIME_FN *report_time_fn)
{
  if (ntp_exec_state == 0)
  {
#ifdef  NTP_DEBUG
    Serial.println("NTP_Start");
#endif

    ntp_report_time_fn = report_time_fn;

    strcpy(ntp_hostname,settings.ntp_hostname);

    NTP_Exec_Set_State(1);
  }
}


void  NTP_Init ()
{
  // Set up pointer to string buffer
  ntp_dns_io.hostname = ntp_hostname;

  // Set up NTP packet
  ntp_udp_tx_packet[0] = 0b00011011;   // LI=00b (leap indicator), Version=011b, Mode=011b

  ntp_udp.begin(5011);
}
