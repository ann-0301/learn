#include <stdio.h>

int main()
{
	FILE *fp = NULL;
	if( (fp = fopen("111", "r" )) == NULL ) {
		unlink("111");
		return;
	}
	char buf[128];
	char str[16] = {0};
	char str1[16] = {0};
	char str2[16] = {0};
	memset( buf, 0, sizeof(buf) );


	while(!feof(fp))
	{
		fscanf(fp,"%[^:]%*c %s",buf,str);
		if (strstr(buf, "os1"))
			strncpy(str1, str, 16);
		else if (strstr(buf, "os2"))
			strncpy(str2, str, 16);
	}
	printf("str1 = %s   str2 = %s \n",str1,str2);


	fclose(fp);
	unlink("111");
	return;
}
