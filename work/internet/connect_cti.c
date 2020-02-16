#include "aes.h"
#include "base64.h"
#include "httpclient.h"
#include "json_gen.h"
#include "time.h"
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/ioctl.h>
#include <net/if.h>

#define PINGTEST			"smarthome.ctdevice.ott4china.com"
static unsigned char key[(KEY_SIZE >> 3)] = "a41eced0871346e6";
#define DBG 0
#define BUFFER_SIZE 4096
#define AES_BLOCK_SIZE 16
static char base64[BUFFER_SIZE];
static int report_success = 0;

typedef struct
{
	char ctei[16];
	char wan_mac[16];
	char ip[18];
	char parentDevMac[13];
	char linkType[2];
	char fmwVersion[16];
	char date[20];
	char rptType[2];
	char sn[64];
	char timeStamp[16];
	char result_code[8];
}REPORT_PARAMETE,*P_REPORT_PARAMETE;

int arp_ip_to_mac(char *hw_address,REPORT_PARAMETE *param)
{
	int ret = 0;
	char buff[128] = {0};	
	char ip_addr[32] = {0};
	char hw_type[8] = {0}; 
	char flags[8] = {0};
	char mac_addr[32] = {0};
	char wan_buff[16]={0};
	char *arry[4] = {0};
	char *mac_arry[6] = {0};

	sscanf(param->ip,"%[^.]",(char*)(&(*arry)+0));
	sscanf(param->ip,"%*[^.].%[^.]",(char*)(&(*arry)+1));
	sscanf(param->ip,"%*[^.].%*[^.].%[^.]",(char*)(&(*arry)+2));
	sprintf(wan_buff,"%s.%s.%s",(char*)(&(*arry)+0),(char*)(&(*arry)+1),(char*)(&(*arry)+2));

	
	FILE *fd = NULL;	
	fd = fopen ("/proc/net/arp", "r"); 
      
    fgets (buff, sizeof(buff)-1, fd);
	while(fgets(buff, sizeof(buff), fd)){
		sscanf (buff, "%s %s %s %s", ip_addr, hw_type, flags, mac_addr);
		if(!strncmp(ip_addr,wan_buff,strlen(wan_buff))){
			//printf("ip_addr = %s,hw_address = %s\n",ip_addr,hw_address);
			//strcpy(hw_address,mac_addr);
			ret = 1;
			break;
		}
	}
	fclose(fd);
	
	if(ret == 1){
		sscanf(mac_addr,"%[^:]",(char*)(&(*mac_arry)+0));
		sscanf(mac_addr,"%*[^:]:%[^:]",(char*)(&(*mac_arry)+1));
		sscanf(mac_addr,"%*[^:]:%*[^:]:%[^:]",(char*)(&(*mac_arry)+2));
		sscanf(mac_addr,"%*[^:]:%*[^:]:%*[^:]:%[^:]",(char*)(&(*mac_arry)+3));
		sscanf(mac_addr,"%*[^:]:%*[^:]:%*[^:]:%*[^:]:%[^:]",(char*)(&(*mac_arry)+4));
		sscanf(mac_addr,"%*[^:]:%*[^:]:%*[^:]:%*[^:]:%*[^:]:%[^:]",(char*)(&(*mac_arry)+5));
		
		sprintf(hw_address,"%s%s%s%s%s%s",(char*)(&(*mac_arry)+0),(char*)(&(*mac_arry)+1),(char*)(&(*mac_arry)+2),(char*)(&(*mac_arry)+3),(char*)(&(*mac_arry)+4),(char*)(&(*mac_arry)+5));
	}
	
	return ret;
}

void getCurrentTime(REPORT_PARAMETE *param) 
{
	char ticks[16] = {0};
	time_t t;
	struct tm * lt;
	time (&t);//
	sprintf(ticks, "%013lu", t);
	strcpy(param->timeStamp,ticks);
	lt = localtime (&t);
	sprintf(param->date,"%04d-%02d-%02d %02d:%02d:%02d",lt->tm_year+1900, lt->tm_mon+1, lt->tm_mday, lt->tm_hour, lt->tm_min, lt->tm_sec);//Êä³ö½á¹û
	return ;
}


void loop_getCurrentTime(int *firstsend_report_nyr,int *firstsend_report_sfm) 
{
	char buff[32]={0};
	time_t t;
	struct tm * lt;
	time (&t);
	lt = localtime (&t);

	memset(buff,0,sizeof(buff));
	sprintf(buff,"%04d%02d%02d",lt->tm_year+1900, lt->tm_mon+1, lt->tm_mday);
	printf("buff = %s\n",buff);
	*firstsend_report_nyr = atoi(buff);
	memset(buff,0,sizeof(buff));
	sprintf(buff,"%02d%02d%02d",lt->tm_hour, lt->tm_min, lt->tm_sec);
	printf("buffsfm = %s\n",buff);
	*firstsend_report_sfm = atoi(buff);	
	return ;
}

void Time_commper(char *time_buff) 
{
	char buff[32]={0};
	time_t t;
	struct tm * lt;
	time (&t);
	lt = localtime (&t);

	memset(time_buff,0,sizeof(time_buff));
	sprintf(time_buff,"%04d",lt->tm_year+1900);
	printf("time_buff = %s\n",time_buff);
	return ;
}

static int get_ifip(char *ifname, char *if_addr)
{
	struct ifreq ifr;
	int skfd = 0;
	if((skfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		printf("%s: open socket error\n", __FUNCTION__);
		return -1;
	}
	strncpy(ifr.ifr_name, ifname, 16);
	if (ioctl(skfd, SIOCGIFADDR, &ifr) < 0) {
		close(skfd);
		return -1;
	}
	strcpy(if_addr, inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));
	close(skfd);
	return 0;
}


int init_param(REPORT_PARAMETE *param,char **v_encrypt)
{
  char wan_mac_buf[16]; 
  char tmp_buff[16];
  char ip_addr[18];

#if DBG
  memset(wan_mac_buf,0,sizeof(wan_mac_buf));
  flash_read_mac(wan_mac_buf); 

  sprintf(param->wan_mac,"%02X%02X%02X%02X%02X%02X",(unsigned char)wan_mac_buf[0],(unsigned char)wan_mac_buf[1],(unsigned char)wan_mac_buf[2],\
  (unsigned char)wan_mac_buf[3],(unsigned char)wan_mac_buf[4],(unsigned char)wan_mac_buf[5]);

  sprintf(param->ctei,"%s%s",nvram_get(CONFIG2_NVRAM, "TUI"),param->wan_mac+5);

  memset(tmp_buff,0,sizeof(tmp_buff));		
  sprintf(tmp_buff, "%s", nvram_get(RT2860_NVRAM, "OperationMode"));
	
	if(tmp_buff[0]=='0')
	{
		get_ifip("br0", param->ip);
	}else if(tmp_buff[0]=='1')
	{
		get_ifip("eth3", param->ip);	
	}else if(tmp_buff[0]=='3')
	{
		if(get_ifip("apcli0", param->ip) == 0)
		{
		
		}
		else if(get_ifip("apclii0", param->ip)==0)
		{
		
		}else
		{
			get_ifip("apcli0", param->ip);
		}
	}

    arp_ip_to_mac(param->parentDevMac,param);

	memset(tmp_buff,0,sizeof(tmp_buff));	
	read_connectgw_file(tmp_buff);

	if(!strcmp(tmp_buff,"yes"))
	{
		strcpy(param->linkType,"1");	
	}else{
		strcpy(param->linkType,"2");
	}

	sprintf(param->fmwVersion, "%s", nvram_get(RT2860_NVRAM, "cwmp:cpe_swver"));
	getCurrentTime(param);
	sprintf(param->sn, "%s", nvram_get(CONFIG2_NVRAM, "sn"));
#else
	sprintf(param->ctei, "%s", "184961298000049");
	sprintf(param->wan_mac, "%s", "649A08000049");
	sprintf(param->ip, "%s", "192.168.17.123");
	sprintf(param->parentDevMac, "%s", "dcfe18be0657");
	sprintf(param->linkType, "%s", "2");
	sprintf(param->fmwVersion, "%s", "ZR-1.0.0");
	getCurrentTime(param);
	//sprintf(param->date, "%s", "2019-07-14 17:25:27");
	if(report_success == 0)
		sprintf(param->rptType, "%s", "1");
	else 
		sprintf(param->rptType, "%s", "2");
	sprintf(param->sn, "%s", "FFFFFFFFFFFFFFFFFFFFFZ649A08000049");
	//sprintf(param->timeStamp, "%s", "0001563013527");

	
#endif
	v_encrypt[0] = param->ctei;
	v_encrypt[1] = param->wan_mac;
	v_encrypt[2] = param->ip;
	v_encrypt[3] = param->parentDevMac;
	v_encrypt[4] = param->linkType;
	v_encrypt[5] = param->fmwVersion;
	v_encrypt[6] = param->date;
	v_encrypt[7] = param->rptType;
	v_encrypt[8] = param->sn;
	v_encrypt[9] = param->timeStamp;
#if DBG
	int i = 0;
	for(i = 0;i<10;i++)
		printf("v_encrypt[%d] = %s\n",i,v_encrypt[i]);
#endif	
	return 0;
}

int ping_userregister_test()
{	
	//char **pptr;
	char time_buff[8]={0};
    struct hostent *hptr;
	int currentsend_report_nyr = 0,currentsend_report_sfm = 0;
    //char   str[32];
    if((hptr = gethostbyname(PINGTEST)) == NULL)
    {
        printf(" gethostbyname error for host:%s\n", PINGTEST);
        return 0;
    }
#if 0
    printf("official hostname:%s\n",hptr->h_name);
    for(pptr = hptr->h_aliases; *pptr != NULL; pptr++)
        printf(" alias:%s\n",*pptr);

    switch(hptr->h_addrtype)
    {
        case AF_INET:
        case AF_INET6:
            pptr=hptr->h_addr_list;
		/*
            for(; *pptr!=NULL; pptr++)
                printf(" address:%s\n",
                       inet_ntop(hptr->h_addrtype, *pptr, str, sizeof(str)));
            printf(" first address: %s\n",
                       inet_ntop(hptr->h_addrtype, hptr->h_addr, str, sizeof(str)));
       */                
        break;
        default:
            printf("unknown address type\n");
        break;
    }
#endif
	Time_commper(time_buff);
	if(strcmp(time_buff,"1970"))	
		return 1;
	else
		return 0;
	
}


int main() 
{
	REPORT_PARAMETE param;
	char *find_p = NULL;
	int ping_result = 0;

	aes_context ctx;
	char *v_encrypt[10] = {0};
	char *k_encrypt[] = {"ctei", "mac", "ip", "parentDevMac", "linkType", "fmwVersion", "date", "rptType", "sn", "timeStamp"};  
	//char *v_encrypt[] = {"184961291234567", "24e271f4d7b0", "192.168.17.1", "010203040506", "1", "11.2.5.21", "2019-04-19 08:53:51", "1", "1231", param.timeStamp};
	char *keys[] = {"manufacturerId", "version", "decryptData", "timeStamp"};
	char *address = "http://smarthome.ctdevice.ott4china.com/boot";
	char *encrypt;
	int len,send_success_flag = 0,send_fail_flag = 0,fail_conut = 0;
	unsigned char *bindata;
	char *input;
	char *values[5];
	int firstsend_report_nyr = 0,currentsend_report_nyr = 0;
	int firstsend_report_sfm = 0,currentsend_report_sfm = 0;
	

while(1)
{
	ping_result = ping_userregister_test();
	if((ping_result == 1))
	{
		if((send_success_flag == 0) && (send_fail_flag == 0))
		{
			memset(&param,0,sizeof(REPORT_PARAMETE));
			init_param(&param,v_encrypt);
			strcpy(param.rptType,"1");
				
			encrypt = genJson(k_encrypt, v_encrypt, sizeof(k_encrypt)/sizeof(char *));
			len = (int)strlen(encrypt);
			printf("encrypt = %s\n",encrypt);
			bindata = encode(&ctx, AES_ENCRYPT, key, (unsigned char *)encrypt, len);
			if (encrypt) 
			{ 
				free(encrypt); 
			}

			memset(base64, 0, BUFFER_SIZE);
			base64_encode(bindata, base64, encode_length(len));
			if (bindata) 
			{ 
				free(bindata);
			}

			values[0] = "A056";
			values[1] = "1.0";
			values[2] = base64;
			values[3] = param.timeStamp;

			input = genJson(keys, values, sizeof(keys)/sizeof(char *));
			  //printf("json:%s, length:%d\n", input, len);  
			http_response_t *response = post(address, input);

			if(input) 
			{ 
				free(input);
			}

			if(response->contents != NULL)
			{
				printf("Repons1:%s\n", response->contents);

				find_p = strstr(response->contents,"code");
				sscanf(find_p+6,"%[^,]",param.result_code);

				if(!strcmp(param.result_code,"0"))
				{
					send_success_flag = 1;
					send_fail_flag = 0;
					report_success = 1;
					loop_getCurrentTime(&firstsend_report_nyr,&firstsend_report_sfm);
					//printf("firstsend_report_nyr = %d\n",firstsend_report_nyr);
					//printf("firstsend_report_sfm = %d\n",firstsend_report_sfm);
				}else{
					report_success = 0;
					send_fail_flag = 1;
					send_success_flag = 0;
				}
				if(response) 
				{
				  	http_response_free(response); 
				}
			}
		}
		if(send_fail_flag == 1)
		{
			fail_conut ++;
			//printf("fail_conut = %d\n",fail_conut);
			if(fail_conut >= 10)
			{
				fail_conut = 0;
				send_fail_flag = 0;
			}
		}else{
			loop_getCurrentTime(&currentsend_report_nyr,&currentsend_report_sfm);
			//printf("currentsend_report_nyr = %d\n",currentsend_report_nyr);
			//printf("currentsend_report_sfm = %d\n",currentsend_report_sfm);
			if((currentsend_report_nyr - firstsend_report_nyr == 1) && (currentsend_report_sfm - firstsend_report_sfm <= 10))//24å°æ—¶ä¸ŠæŠ¥
			{
				printf("24 report\n");
				send_success_flag = 0;
				send_fail_flag = 0;
			}
			fail_conut = 0;
		}
		
	}else{
			printf(" ping fail\n");
			send_success_flag = 0;
			fail_conut = 0;
			send_fail_flag = 0;
		}
	  sleep(1);
  }	
  return 0;
}

