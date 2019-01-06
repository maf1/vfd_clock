
/*
  DNS lookup host by name

  Note: macro DNS_DEBUG is already defined somewhere in the Arduino Core for ESP32, so we use
  DNS_LOOKUP_DEBUG istead.
*/


extern "C"
{
#include "lwip/dns.h"
}


void  DNS_Host_By_Name_Complete (DNS_HOST_BY_NAME_IO *io)
{
  io->completed = true;
}


void  DNS_Host_By_Name_CB (const char *hostname, const ip_addr_t *ipaddr, void *callback_arg)
{
  DNS_HOST_BY_NAME_IO  *io;

  io = (DNS_HOST_BY_NAME_IO*)callback_arg;

#ifdef  DNS_LOOKUP_DEBUG
  Serial.print("DNS_Host_By_Name_CB: ");
  Serial.print(hostname);
  Serial.print(" -> ");
#endif

  if (ipaddr)
  {
    io->res_ad = ipaddr->u_addr.ip4.addr;

#ifdef  DNS_LOOKUP_DEBUG
    Serial.print("IP address ");
    Serial.println(io->res_ad);
#endif
  }
  else
  {
#ifdef  DNS_LOOKUP_DEBUG
    Serial.println("no IP address");
#endif

    io->res_ad = (uint32_t)0;
  }

  DNS_Host_By_Name_Complete(io);
}


boolean  DNS_Host_By_Name_Has_Completed (DNS_HOST_BY_NAME_IO *io)
{
  return io->completed;
}


void  DNS_Host_By_Name_Init (DNS_HOST_BY_NAME_IO *io)
{
  err_t err;

#ifdef  DNS_LOOKUP_DEBUG
  Serial.print("DNS_Host_By_Name_Init: ");
#endif

  io->completed = false;

  // Note: this function returns ERR_VAL if there's no network connection, or the network
  // connection has no associated DNS server IP addresses.
  err = dns_gethostbyname(io->hostname,&io->addr,&DNS_Host_By_Name_CB,io);
#ifdef  DNS_LOOKUP_DEBUG
  Serial.printf("err %d, ",err);
#endif
  if (err == ERR_INPROGRESS)
  {
    // Pending operation

#ifdef  DNS_LOOKUP_DEBUG
    Serial.println("pending");
#endif
  }
  else
  {
#ifdef  DNS_LOOKUP_DEBUG
    Serial.print("done -> ");
#endif

    if (err == ERR_OK)
    {
      // Address can be zero or non-zero
      io->res_ad = io->addr.u_addr.ip4.addr;

#ifdef  DNS_LOOKUP_DEBUG
      Serial.print("IP address ");
      Serial.println(io->res_ad);
#endif
    }
    else
    {
      io->res_ad = (uint32_t)0;

#ifdef  DNS_LOOKUP_DEBUG
      Serial.println("no IP address");
#endif
    }

    DNS_Host_By_Name_Complete(io);
  }
}

