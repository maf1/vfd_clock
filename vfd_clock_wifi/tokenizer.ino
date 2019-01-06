
/*
  Tokenizer
*/


void  Tok_Buf_Reset (TOK *t)
{
    t->buf_si = 0;
}


boolean  Tok_Buf_Store (TOK *t, char c)
{
    if (t->buf_si < (sizeof(t->buf)-1))
    {
        t->buf[t->buf_si] = c;
        t->buf_si++;

        return true;
    }
    else
        return false;
}


void  Tok_Buf_Terminate (TOK *t)
{
    t->buf[t->buf_si] = 0;
}


void  Tok_Init (TOK *t, char *s)
{
    t->s  = s;
    t->fi = 0;
}


char  Tok_Upcase (char c)
{
    if ((c >= 'a') && (c <= 'z')) c = c + 'A' - 'a';
    return c;
}


boolean  Tok_Parse_Val (TOK *t)
{
    char      c;
    uint8_t   radix;              // Radix (2, 10, 16)
    uint32_t  cnt;                // Characters to parse
    uint32_t  n;
    uint32_t  val;                // Value
    uint32_t  prev_val;           // Previous value (required for overflow detection)

    // Get the radix and the number of characters to parse
    cnt = t->buf_si - 1;
    c   = Tok_Upcase(t->buf[cnt]);
    if (c == 'B') radix = 2; else
    if (c == 'D') radix = 10; else
    if (c == 'H') radix = 16; else
    {
        cnt++; radix = 10;
    }

    if (cnt == 0)
    {
        // First character is invalid
        return false;
    }

    // Parse the characters and calculate the value
    val = 0;
    for (n = 0; n < cnt; n++)
    {
        uint32_t  digit;

        c = Tok_Upcase(t->buf[n]);
        if (c != '_')
        {
            if ((c >= '0') && (c <= '9')) digit = c - '0'; else
            if ((c >= 'A') && (c <= 'F')) digit = c + 10 - 'A'; else
            {
                // Invalid character
                return false;
            }

            if (digit >= radix)
            {
                // Invalid character
                return false;
            }

            prev_val = val;
            val = val * radix + digit;
            if (val < prev_val)
            {
                // Overflow
                return false;
            }
        }
    }

    // Return successfully
    t->val = val;
    return true;
}


boolean  Tok_Fetch (TOK *t)
{
    char    c;

    for (;;)
    {
        c = t->s[t->fi];

        if ((c == 32) || (c == 9))
        {
            // Skip space and tab characters
            t->fi++;
        }
        else
        {
            t->start_index = t->fi;
            break;
        }
    }

    if ((c == 13) || (c == 10) || (c == '#') || (c == 0))
    {
        // Report end-of-line token
        t->id = TOK_ID_EOL;
        return true;
    }
    else
    if ((c >= '0') && (c <= '9'))
    {
        // Reset the text buffer
        Tok_Buf_Reset(t);

        do
        {
            // Store the character in the text buffer
            if (!Tok_Buf_Store(t,c)) return false;

            t->fi++;
            c = t->s[t->fi];
        }
        while (
               ((c >= 'A') && (c <= 'Z')) ||
               ((c >= 'a') && (c <= 'z')) ||
               ((c >= '0') && (c <= '9')) ||
                (c == '_')
              );

        // Terminate the string in the text buffer
        Tok_Buf_Terminate(t);

        // Parse the number
        if (!Tok_Parse_Val(t)) return false;

        // Report number token
        t->id = TOK_ID_NUMBER;
        return true;
    }
    else
    if (
        ((c >= 'A') && (c <= 'Z')) ||
        ((c >= 'a') && (c <= 'z')) ||
         (c == '_') || (c == '?')
       )
    {
        // Reset the text buffer
        Tok_Buf_Reset(t);

        do
        {
            // Store the character in the text buffer
            if (!Tok_Buf_Store(t,c)) return false;

            t->fi++;
            c = t->s[t->fi];
        }
        while (
               ((c >= 'A') && (c <= 'Z')) ||
               ((c >= 'a') && (c <= 'z')) ||
               ((c >= '0') && (c <= '9')) ||
                (c == '_')
              );

        // Terminate the string in the text buffer
        Tok_Buf_Terminate(t);

        // Report label token
        t->id = TOK_ID_LABEL;
        return true;
    }
    else
    if (c == '"')
    {
        t->fi++;

        // Reset the text buffer
        Tok_Buf_Reset(t);

        for (;;)
        {
            c = t->s[t->fi];
            t->fi++;

            if (c == '"') break;

            if (c < 32)
            {
                // Invalid character
                return false;
            }

            // Store the character in the text buffer
            if (!Tok_Buf_Store(t,c)) return false;
        }

        // Terminate the string in the text buffer
        Tok_Buf_Terminate(t);

        // Report label token
        t->id = TOK_ID_STRING;
        return true;
    }
    else
    if ((c >= 33) && (c <= 126))
    {
        // Reset the text buffer
        Tok_Buf_Reset(t);

        // Store the character in the text buffer
        if (!Tok_Buf_Store(t,c)) return false;

        t->fi++;

        // Terminate the string in the text buffer
        Tok_Buf_Terminate(t);

        // Report label token
        t->id = TOK_ID_CHAR;
        return true;
    }
    else
        // Invalid character
        return false;
}


boolean  Tok_Fetch_EOL (TOK *t)
{
    if (!Tok_Fetch(t)) return false;

    return (t->id == TOK_ID_EOL) ? true : false;
}
