#include "common.h"
struct s_list_t *list_create(){

    struct s_list_t *list=(struct s_list_t *)calloc(sizeof(struct s_list_t),1);
    return list;
}
void list_destroy(struct s_list_t *list,void (*each)(void *item,size_t idx)){
    struct list_node_t *node=list->head;
    size_t idx=0;
    while(node!=NULL){
        void *item=node->value;
        list->head=node->next;
        //free(item);
        if(each)
            each(item,idx);
        free(node);
        node=list->head;
        idx++;
    }
    free(list);

}
size_t list_add(struct s_list_t *list,void *item){
    if(item==NULL)
        return list->size;
    struct list_node_t *node=calloc(sizeof(struct list_node_t),1);
    node->value=item;
    if(list->size==0){
        list->head=node;
        list->last=node;
    }
    else{
        list->last->next=node;
        list->last=node;
    }
    return ++list->size;
}
int list_remove(struct s_list_t *list,void *item){
    if(list->size==0||item==NULL){
        return -1;
    }
    struct list_node_t *cur=list->head;
    struct list_node_t *prev=NULL;

    size_t idx=0;
    while(cur!=NULL){

        //struct list_node_t *next=(*node)->next;
        if(cur->value==item){
            if(cur->next==NULL){
                //prev->next=cur->next;
                list->last=prev;
            }
            if(prev==NULL){
                list->head=cur->next;
            }
            else{
                prev->next=cur->next;
            }
            free(cur);
            list->size--;
            return idx;
        }
        idx++;
        prev=cur;
        cur=cur->next;

    }
    return -1;


}
void *list_remove_at(struct s_list_t *list,size_t index){
    if(list->size==0){
        return NULL;
    }
    struct list_node_t *cur=list->head;
    struct list_node_t *prev=NULL;
    void *value=NULL;
    size_t idx=0;
    while(cur!=NULL){

        //struct list_node_t *next=(*node)->next;
        if(idx==index){
            value=cur->value;
            if(cur->next==NULL){
                //prev->next=cur->next;
                list->last=prev;
            }
            if(prev==NULL){
                list->head=cur->next;
            }
            else{
                prev->next=cur->next;
            }
            free(cur);
            list->size--;
            return value;
        }
        idx++;
        prev=cur;
        cur=cur->next;

    }
    return NULL;


}
int list_contain(struct s_list_t *list,void *item){

    if(list->size==0){
        return FALSE;
    }
    struct list_node_t *node=list->head;

    while(node!=NULL){
        void *_item=node->value;
        //struct list_node_t *next=(*node)->next;
        if(item==_item){
            return TRUE;
        }

        node=node->next;

    }
    return FALSE;
}
void list_each(struct s_list_t *list,int (*cb)(void *item,size_t index,void *rdata),void *data){
    if(list->size==0){
        return;
    }
    struct list_node_t *node=list->head;
    size_t idx=0;

    while(node!=NULL){
        void *_item=node->value;
        //struct list_node_t *next=(*node)->next;
        cb(_item,idx,data);

        node=node->next;
        idx++;

    }

}
void *list_get(struct s_list_t *list,size_t index){
    if(list->size==0){
        return NULL;
    }
    struct list_node_t *node=list->head;
    size_t idx=0;

    while(node!=NULL){
        if(idx==index){
            void *_item=node->value;
            //struct list_node_t *next=(*node)->next;
            return _item;
        }
        node=node->next;
        idx++;

    }
    return NULL;

}
size_t list_size(struct s_list_t *list){
    return list->size;
}
