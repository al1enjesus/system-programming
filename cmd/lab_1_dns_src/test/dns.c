
/*************************************************************************
   LAB 1                                                                

    Edit this file ONLY!

*************************************************************************/



#include <stdio.h>
#include "dns.h"
#include "hashtable.h"

DNSHandle InitDNS( )
{
    HashTable *hashTable = HashTableInit();
    return (DNSHandle) hashTable;
}

void LoadHostsFile( DNSHandle hDNS, const char* hostsFilePath )
{
    HashTable* hT = (HashTable*) hDNS;

    FILE* fInput = NULL;
    fInput = fopen(hostsFilePath, "r");
    char buffer[256];
    buffer[255] = '\0';
    while((fgets(buffer, 256, fInput))!=NULL)
    {
        char ip[21], domain[100];
        ip[20] = '\0'; domain[99] = '\0';
        sscanf(buffer, "%s    %s", &ip, &domain);
        HashTableInsert(hT, domain, ip);
    }

	fclose(fInput);
}

IPADDRESS DnsLookUp( DNSHandle hDNS, const char* hostName )
{
    HashTable* hT = (HashTable*) hDNS;
    char * ip = HashTableSearch(hT, hostName);
    if (ip == NULL){
        return INVALID_IP_ADDRESS;
    }

    uint32_t ip_int = 0;
    uint32_t ip1, ip2, ip3, ip4;
    char * buffer;
    sscanf(ip, "%d.%d.%d.%d %s", &ip1, &ip2, &ip3, &ip4, &buffer);
    ip_int = ( ip1 & 0xFF ) << 24 |
             ( ip2 & 0xFF ) << 16 |
             ( ip3 & 0xFF ) << 8  |
             ( ip4 & 0xFF );
    return ip_int;
}

void ShutdownDNS( DNSHandle hDNS )
{
    HashTable *hT = (HashTable*) hDNS;
    HashTableRemove(hT);
}
