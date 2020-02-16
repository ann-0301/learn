#include <string.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <linux/if.h>
#include <linux/if_bridge.h>
#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>
#include <stdio.h>

static int getHostname(char **ppStart, unsigned long *size,unsigned char *mac_hostname)
{
	struct dhcpOfferedAddr {
		u_int8_t chaddr[16];
		u_int32_t yiaddr;       /* network order */
		u_int32_t expires;      /* host order */
		u_int32_t interfaceType;
		u_int8_t hostName[64];
		unsigned int active_time;
	};

	struct dhcpOfferedAddr entry;
		
	if ( *size < sizeof(entry) )
		return -1;

	entry = *((struct dhcpOfferedAddr *)*ppStart);
	*ppStart = *ppStart + sizeof(entry);
	*size = *size - sizeof(entry);
		
	if (entry.expires == 0)
		return 0;
    //star: conflict ip addr will not be displayed on web
	if(entry.chaddr[0]==0&&entry.chaddr[1]==0&&entry.chaddr[2]==0
	&&entry.chaddr[3]==0&&entry.chaddr[4]==0&&entry.chaddr[5]==0)
		return 0;
	
	printf("%02x%02x%02x%02x%02x%02x-----%s\n",entry.chaddr[0],entry.chaddr[1],entry.chaddr[2],entry.chaddr[3],entry.chaddr[4],entry.chaddr[5], entry.hostName);
	sprintf(mac_hostname + strlen(mac_hostname),"\"%02x%02x%02x%02x%02x%02x\":\"%s\",",entry.chaddr[0],entry.chaddr[1],entry.chaddr[2],entry.chaddr[3],entry.chaddr[4],entry.chaddr[5], entry.hostName);
	//strcpy((char *)mac_hostname, (char *)entry.hostName);
		return 0;
}

#define DHCPD_LEASE  "./udhcpd.leases"
static char * get_mac_hostname()
{
	struct stat status;
	FILE *fp = NULL;
	char *buf = NULL, *ptr;
	int ret,pid;
	unsigned long temp_size = 0;
/*	
	pid = read_pid("/var/run/udhcpd.pid");
	if (pid > 0)
		kill(pid, SIGUSR1);
	usleep(1000);
*/				
	if (stat(DHCPD_LEASE, &status) < 0)
		return 0;
	char * mac_hostname = NULL;
	printf("---%s(%d)----date_size=%ld\n",__FUNCTION__,__LINE__,status.st_size);
	// read DHCP server lease file
	buf = malloc(status.st_size);
	mac_hostname =  malloc(status.st_size);
	printf("---%s(%d)----date_size=%d\n",__FUNCTION__,__LINE__,sizeof(*mac_hostname));
	if (buf == NULL)
		goto end_dhcp;
	fp = fopen(DHCPD_LEASE, "r");
	if (fp == NULL)
		goto end_dhcp;
	memset(buf,0,status.st_size);
	fread(buf, 1, status.st_size, fp);
	fclose(fp);

	ptr = buf;
	temp_size = status.st_size;
	sprintf(mac_hostname,"{");
	while (1) {
		ret = getHostname(&ptr, &temp_size,mac_hostname);
		if (ret == 0)
			continue;
		else
			break;
	}
	sprintf(mac_hostname + strlen(mac_hostname),"}");
end_dhcp:
	if (buf)
		free(buf);
	
	return mac_hostname;
}
int main()
{
	char mac[64] = {0};
//	char hostname[4096] = {0};
	unsigned char *hostname = NULL;
	hostname = get_mac_hostname();
	printf("---%s(%d)----hostname=%s\n",__FUNCTION__,__LINE__,hostname);
	printf("111\n");
}
