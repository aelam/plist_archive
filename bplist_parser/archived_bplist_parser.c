//
//  bplist_parser.c
//  ideviceruntest
//
//  Created by Wang Lun on 2020/9/28.
//  Copyright © 2020 JonGabilondo. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>

#include "archived_plist_parser.h"

void printNode(plist_t node) {
    plist_type t = plist_get_node_type(node);
    if (t == PLIST_UINT) {
        uint32_t v = 0;
        plist_get_uint_val(node, &v);
        printf("%d", v);
    } else if (t == PLIST_REAL) {
        double v = 0;
        plist_get_real_val(node, &v);
        printf("%f", v);
    } else if (t == PLIST_DICT) {
        printf("A DIC");
    } else if (t == PLIST_ARRAY) {
        printf("AN ARRAY");
    } else if (t == PLIST_STRING) {
        char *v;
        plist_get_string_val(node, &v);
        printf("%s", v);
    } else if (t == PLIST_BOOLEAN) {
        int v;
        plist_get_bool_val(node, &v);
        printf("%d", v);
    }
}

void print_plist(plist_t pl) {

    char* xml = NULL;

    uint32_t xlen = 0;

    plist_to_xml(pl, &xml, &xlen);

    if (xml) {

        printf("%s\n", xml);

        free(xml);

    }

}


// Object Key
char *parse_bplist_object_key(plist_t objects, plist_t current_node_keys, int i) {
    plist_t current_node_key_key = plist_array_get_item(current_node_keys, i);
    
    plist_t UID = plist_dict_get_item(current_node_key_key, "CF$UID");
    uint32_t data_type_uid_val = 0;
    plist_get_uint_val(UID, &data_type_uid_val);
    plist_t t = plist_array_get_item(objects, data_type_uid_val);
    char *result = NULL;
    plist_get_string_val(t, &result);
    
    return result;
}

// Object Value
plist_t parse_bplist_object_value(plist_t objects, plist_t current_node_objects, int i) {
    plist_t current_node_object = plist_array_get_item(current_node_objects, i);

    plist_t UID = plist_dict_get_item(current_node_object, "CF$UID");
    uint32_t data_type_uid_val;
    plist_get_uint_val(UID, &data_type_uid_val);
    plist_t t = plist_array_get_item(objects, data_type_uid_val);
    return plist_copy(t);
}

plist_t parse_bplist_object(plist_t objects, plist_t current_node) {
    plist_t node = NULL;
    
    plist_t data_uid = plist_access_path(current_node, 1, "CF$UID");
    uint32_t data_uid_val;
    plist_get_uint_val(data_uid, &data_uid_val);

    plist_t data = plist_array_get_item(objects, data_uid_val);

    // parse $class
    plist_t data_type_uid = plist_access_path(data, 2, "$class", "CF$UID");
    uint32_t data_type_uid_val;
    plist_get_uint_val(data_type_uid, &data_type_uid_val);
    
    // get class by class id
    plist_t data_type_class = plist_array_get_item(objects, data_type_uid_val);
    plist_t data_type_classname = plist_dict_get_item(data_type_class, "$classname");
    char * data_type_classname_val;
    plist_get_string_val(data_type_classname, &data_type_classname_val);

    if(strcmp(data_type_classname_val, "NSMutableDictionary") == 0 ||
       strcmp(data_type_classname_val, "NSDictionary") == 0
       ) {
        node = plist_new_dict();
        
        plist_t data_keys = plist_access_path(data, 1, "NS.keys");
        plist_t data_objects = plist_access_path(data, 1, "NS.objects");
        uint32_t size = plist_array_get_size(data_keys);
        
        for (int i = 0; i < size; i++) {
            char *key = parse_bplist_object_key(objects, data_keys, i);
            plist_t value = parse_bplist_object_value(objects, data_objects, i);
            
            plist_dict_set_item(node, key, value);
            printf("key: %s, value:", key);
            printNode(value);
            printf("\n");
            
        }
    }
    
    return node;
}

plist_t parse_archived_plist(plist_t bplist) {
    plist_t new_plist = NULL;
    
    // root type
    plist_t root_uid = plist_access_path(bplist, 3, "$top", "root", "CF$UID");
    plist_t objects = plist_dict_get_item(bplist, "$objects");

    // get root Id
    uint64_t root_uid_val;
    plist_get_uint_val(root_uid, &root_uid_val);

    
    //
    // get root class id
    plist_t data = plist_array_get_item(objects, root_uid_val);
    plist_t data_type_uid = plist_access_path(data, 2, "$class", "CF$UID");
    double data_type_uid_val;
    plist_get_real_val(data_type_uid, &data_type_uid_val);
    
    // get class by class id
    plist_t data_type_class = plist_array_get_item(objects, data_type_uid_val);
    plist_t data_type_classname = plist_dict_get_item(data_type_class, "$classname");
    char * data_type_classname_val;
    plist_get_string_val(data_type_classname, &data_type_classname_val);

    if(strcmp(data_type_classname_val, "NSMutableArray") == 0 ||
       strcmp(data_type_classname_val, "NSArray") == 0
       ) {
        // EQUAL!!
        new_plist = plist_new_array();
    } else {
        return NULL;
    }
    
    
    plist_t data_array_objects = plist_access_path(data, 1, "NS.objects");
    uint32_t objects_size = plist_array_get_size(data_array_objects);

    for (uint32_t i = 0; i < objects_size; i++ ) {
        plist_t current_node = plist_array_get_item(data_array_objects, i);
        plist_t item_node = parse_bplist_object(objects, current_node);
        plist_array_append_item(new_plist, item_node);
//        print_plist(item_node);
    }

//    print_plist(new_plist);

    return new_plist;
}


