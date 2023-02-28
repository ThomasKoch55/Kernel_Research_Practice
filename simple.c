#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include <arpa/inet.h>


// https://www.digitalocean.com/community/tutorials/trie-data-structure-in-c-plus-plus

// We will create a 256-ary trie
#define N 256

typedef struct TrieNode TrieNode;

    // must declare a global variable root (its children will be the first octet)
struct TrieNode *root = NULL;

struct TrieNode
{
    //the trie node struct
    // each node will have n children

    unsigned int data[4];
    TrieNode *children[N];
    int next_hop_interface;
    unsigned int SNM;
    int level;
    
};

TrieNode* make_trienode()
{
    TrieNode *node = malloc(sizeof(TrieNode));
    for (int i = 0; i<N; i++)
    {
        node -> children[i] = NULL;
    }
    //for (int i=0; i < 4; i++)
    //{
    //    node->data[i] = data[i];
    //}
    
    return node;
}

//Recursively frees the node and all its kids kids kids nodes 
void free_trienode(TrieNode *node)
{
    for (int i = 0; i<N; i++)
    {
        if (node -> children[i] != NULL)
        {
            free_trienode(node -> children[i]);
        }
    else
    {
        continue;
    }
    }
    free(node);

}



unsigned int prefix_to_mask(int prefix)
{
    unsigned int ret = 0xffffffff;

    if (prefix == 0)
    {
        ret = 0;
    }

    else
    {
        ret <<= (32 - prefix);
    } 
    return ret;
}




int insert_trie2(unsigned int net_addr, int next_hop_int)
{
    
    int i;
    //unsigned char because its 1 byte
    unsigned char octet;
    struct TrieNode *new_node, *cur_node;

    if (root == NULL) //needs error handling
    {
        root = make_trienode();
    }

    cur_node = root;
    for(i=0; i<4; i++)
    {
        
        octet = net_addr >> ((3 - i) * 8);
        
        if (cur_node->children[octet] == NULL) //needs error handling
        {
            new_node = make_trienode();
            new_node->next_hop_interface = next_hop_int;
            new_node->level = i+1; // i + 1 because root is techincally level 0
            cur_node->children[octet] = new_node;
            cur_node = new_node;
        }
        else
        {
            cur_node = cur_node->children[octet];
        }

    }
    
    return 1;
}




int insert_route(char *ip_dotNotn, char *SNM, int next_hop_int)
{
    //ip format = "10.2.3.4"
    //SNM format = "/24"
    //NHI format = (int) 1

    unsigned int ip, mask, net_addr;

    //convert string ip to binary addy in network byte order (big edian)
    //htonl may not be needed since inet_addr says it returns in network byte order
    ip = htonl(inet_addr(ip_dotNotn));
    //convert SNM to int then to binary mask
    mask = prefix_to_mask(atoi(&SNM[1]));

    //by anding the ip and mask, we will get the subnet address
    //ex: ip = 11111111.11110000.11100000.00000000
    //  mask = 11111111.10000000.00000000.00000000
    //   res = 11111111.10000000.00000000.00000000

    net_addr = ip & mask;

    insert_trie2(net_addr, next_hop_int);

    return 1;
}





//searches each layer by index of ipAddr[x], inserting when it finds a null
TrieNode* insert_trie(TrieNode *root, int *ipAddr)
{
    TrieNode *temp = root;

    
     
     for (int i=0; i<4; i++)
     {
        if (i==0)
        {
            if(temp->children[ipAddr[i]] == NULL)
            {
                temp->children[ipAddr[i]] = make_trienode(ipAddr);
                temp->children[ipAddr[i]]->level = 1;
            }
            else continue;
        }
        
        if (i==1)
        {
            if (temp->children[ipAddr[0]]->children[ipAddr[i]] == NULL)
            {
                temp->children[ipAddr[0]]->children[ipAddr[i]] = make_trienode(ipAddr);
                temp->children[ipAddr[0]]->children[ipAddr[i]]->level = 2;
            }
            else
                continue;
        }
        
        if (i == 2)
        {
            if (temp->children[ipAddr[0]]->children[ipAddr[1]]->children[ipAddr[i]] == NULL)
            {
                temp->children[ipAddr[0]]->children[ipAddr[1]]->children[ipAddr[i]] = make_trienode(ipAddr);
                temp->children[ipAddr[0]]->children[ipAddr[1]]->children[ipAddr[i]]->level = 3;
            }
            else
                continue;
        }

        if (i == 3)
        {
            if (temp->children[ipAddr[0]]->children[ipAddr[1]]->children[ipAddr[2]]->children[ipAddr[i]] == NULL)
            {
                temp->children[ipAddr[0]]->children[ipAddr[1]]->children[ipAddr[2]]->children[ipAddr[i]] = make_trienode(ipAddr);
                temp->children[ipAddr[0]]->children[ipAddr[1]]->children[ipAddr[2]]->children[ipAddr[i]]->level = 4;
            }
            else
                continue;
        } 
     }

     return root;
}

unsigned char level_octet[4];

void printTrie(TrieNode *node)
{
    
    if (node == NULL)
    {
        return;
    }

    for (int i=0; i<256; i++)
    {
        if(node->children[i] != NULL)
        {
            struct TrieNode *next_node;
            next_node = node->children[i];

            level_octet[next_node->level-1] = i;
            
            if(next_node->level == 4)
            {
                printf("%d.%d.%d.%d, nextHop: %d\n", level_octet[0],level_octet[1],level_octet[2],level_octet[3], next_node->next_hop_interface);
            }
            printTrie(node->children[i]);
            
        }
    }   
}


int main()
{
    
    
    int rootip[4] = {0, 0, 0, 0};
    TrieNode *rootnode;
    rootnode = make_trienode(rootip);

    int testip[4] = {192,0,0,0};
    TrieNode *testnode;
    testnode = make_trienode(testip);

    insert_route("192.168.1.3", "/16", 1);
    insert_route("192.168.1.1", "/24", 2);
    //test adding same netwk addy
    insert_route("192.168.1.2", "/24", 2);
    insert_route("192.168.1.6", "/24", 2);
    insert_route("192.168.2.5", "/24", 2);
    insert_route("69.117.50.10", "/21", 3);

    printTrie(root);
    




    







    //printf("data of rootnode: %d\n", rootnode->data[1]);

    //printf("data[1] of kid[1]: %d\n", rootnode->children[1]->data[1]);
    //printf("data[0] of kid[192]: %d\n level: %d\n", rootnode->children[192]->data[0], rootnode->children[192]->level);
    //printf("data[0] of kid[192]->kid[168]: %d\n", rootnode->children[192]->children[168]->data[0]);
    
    
    
    /*
    unsigned char ip_parsed[sizeof(struct in_addr)];
    char ip_parsed_printable[INET_ADDRSTRLEN];


    inet_pton(AF_INET, testIP, ip_parsed);
    inet_ntop(AF_INET, ip_parsed, ip_parsed_printable, INET_ADDRSTRLEN);
    printf("result: %s", ip_parsed_printable);
    */
    
    return 0;
}


//functions that convert ips to ints: inet_pton
//split on / 

