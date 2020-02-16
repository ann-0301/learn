// AAAAAAAAAAAA -> aa:aa:aa:aa:aa:aa
void MacChangeFromAToa(char *srcmac, char *dstmac)
{
  int i, j = 0;
  int k = 2;

  for (i = 0; i < 17; i++)
  {
    if (i == k)
    {
      dstmac[i] = ':';
      k = k+3;
      j = j+1;
    }
    else
    {
        if(srcmac[i-j] >= 'A' && srcmac[i-j] <= 'Z')
        {
          dstmac[i] = srcmac[i-j] + 32;
        }
        else
        {
          dstmac[i] = srcmac[i-j];
        }
    }
  }
}

// aa:aa:aa:aa:aa:aa -> AAAAAAAAAAAA
void MacChangeFromaToA(char *srcmac, char *dstmac)
{
  int i, j = 0;
  for (i = 0; srcmac[i]; i++)
  {
    if (srcmac[i] <= 'z' && srcmac[i] >= 'a')
    {
      dstmac[j] = (srcmac[i] - 'a') + 'A';
      j++;
    }
    else if (srcmac[i] == ':')
    {
      continue;
    }
    else
    {
      dstmac[j] = srcmac[i];
      j++;
    }
  }
}
