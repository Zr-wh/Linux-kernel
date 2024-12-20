#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TABLE_SIZE 100

// 节点结构体
typedef struct Node {
    char mac_address[6]; // MAC 地址字符串，假设最大长度为 17
    char valie[6];
    struct Node* prev;
    struct Node* next;
} Node;

// 哈希表结构体
typedef struct {
    Node* table[TABLE_SIZE]; // 哈希表数组，每个桶是一个双向链表的头节点
} HashMap;

// 哈希函数
unsigned int hash_function(const char* mac) {
    unsigned int hash = 0;
    while (*mac) {
        hash = hash * 31 + *mac;
        mac++;
    }
    return hash % TABLE_SIZE;
}

// 初始化哈希表
HashMap* init_hash_map() {
    HashMap* map = (HashMap*)malloc(sizeof(HashMap));
    for (int i = 0; i < TABLE_SIZE; i++) {
        map->table[i] = NULL;
    }
    return map;
}

// 插入 MAC 地址到哈希表
void insert_mac(HashMap* map, const char* mac) {
    unsigned int index = hash_function(mac);
    Node* new_node = (Node*)malloc(sizeof(Node));
    strcpy(new_node->mac_address, mac);
    new_node->prev = NULL;
    new_node->next = map->table[index];
    if (map->table[index] != NULL) {
        map->table[index]->prev = new_node;
    }
    map->table[index] = new_node;
}

// 查找 MAC 地址
Node* find_mac(HashMap* map, const char* mac) {
    unsigned int index = hash_function(mac);
    Node* current = map->table[index];
    while (current != NULL) {
        if (strcmp(current->mac_address, mac) == 0) {
            return current;
        }
        current = current->next;
    }
    return NULL; // 未找到
}

// 删除 MAC 地址
void delete_mac(HashMap* map, const char* mac) {
    unsigned int index = hash_function(mac);
    Node* current = map->table[index];
    while (current != NULL) {
        if (strcmp(current->mac_address, mac) == 0) {
            if (current->prev != NULL) {
                current->prev->next = current->next;
            } else {
                map->table[index] = current->next;
            }
            if (current->next != NULL) {
                current->next->prev = current->prev;
            }
            free(current);
            return;
        }
        current = current->next;
    }
}

int main() {
    HashMap* map = init_hash_map();

    // 插入 MAC 地址到哈希表
    insert_mac(map, "00:11:22:33:44:55");
    insert_mac(map, "AA:BB:CC:DD:EE:FF");

    // 删除 MAC 地址
    delete_mac(map, "00:11:22:33:44:55");

    // 查找 MAC 地址
    const char* mac_to_find = "AA:BB:CC:DD:EE:FF";
    Node* found_node = find_mac(map, mac_to_find);

    if (found_node) {
        printf("MAC address %s found in hash table.\n", found_node->mac_address);
    } else {
        printf("MAC address not found in hash table.\n");
    }

    // 释放哈希表内存
    for (int i = 0; i < TABLE_SIZE; i++) {
        Node* current = map->table[i];
        while (current != NULL) {
            Node* temp = current;
            current = current->next;
            free(temp);
        }
    }
    free(map);

    return 0;
}