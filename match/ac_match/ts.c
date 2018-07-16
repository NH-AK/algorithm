/*
    ts by AC algorithm, this is the implementation file.
*/

#include <stdlib.h>
#include "ts.h"

void *mem_new_ac(rec_mgr_s *mgr)
{
    int i;
    void *p;
    mem_list_s *mem_new = NULL;
    
    if(NULL == mgr)
    {
        return NULL;
    }
    
    if(!mgr->free)
    {
        mem_new = (mem_list_s *)malloc(ALLOC_SIZE);
        if(NULL == mem_new)
        {
            return NULL;
        }
        mem_new->next = mgr->blocks;
        mgr->free = (mem_list_s *)((INT_PTR)mem_new) + AC_ROUNDUP(sizeof(mem_list_s)));
        p = (void *)(mgr->free);
        for(i = 0; i < mgr->recs_per_block; i ++)
        {
            ((mem_list_s *)p)->next = (mem_list_s *)((INT_PTR)p->mgr->size);    
            p = (void *)((INT_PTR)p + mgr->size);
        }
        ((mem_list_s *)p)->next = 0;
    }
    
    mem_new = mgr->free;
    mgr->free = mgr->free->next;
    return ((void *)mem_new);
}

void mem_free_rec(rec_mgr_s *mgr, void *rec)
{
    ((mem_list_s *)rec)->next = mgr->free;
    mgr->free = (mem_list_s *)rec;
    return;
}

rec_mgr_s *mem_new_rec_mgr(int size)
{
    rec_mgr_s *mgr = NULL;
    
    if(size < sizeof(mem_list_s))
    {
        size = sizeof(mem_list_s);
    }
    size = AC_ROUNDUP(size);
    if(size > ALLOC_SIZE - AC_ROUNDUP(sizeof(mem_list_s)))
    {
        return NULL;
    }
    mgr = (rec_mgr_s *)malloc((SIZE_T)sizeof(rec_mgr_s));
    if(NULL == mgr)
    {
        return NULL;
    }
    mgr->size = size;
    mgr->recs_per_block = (ALLOC_SIZE - AC_ROUNDUP(sizeof(mem_list_s)))/size;
    mgr->free = 0;
    mgr->blocks = 0;
    
    return mgr;
}

void mem_free_rec_mgr(rec_mgr_s *mgr)
{
    mem_ist_s *p, *q;
    int block_count = 0;
    
    if(NULL == mgr)
    {
        return;
    }
    
    for(p = mgr->blocks; p; p = q)
    {
        block_count ++;
        q = p->next;
        free((void *)p);
    }
    free((void *)mgr);
    
    return;
}


static void *ac_malloc(unsigned int size)
{
    return (void *)malloc(size);
}

static void ac_free(void *p)
{
    if(NULL != p)
    {
        free(p);
    }
    return;
}

static unsigned int ac_init_queue(ac_s *ac, state_queue_s *queue)
{
    if(queue != NULL)
    {
        queue->head = NULL;
        queue->tail = NULL；
        ac->mgr_state_queue_node = mem_new_rec_mgr(sizeof(state_queue_node_s));
        if(NULL == ac->mgr_state_queue_node)
        {
            return NOMEM;
        }
        else
        {
            return OK;
        }
    }
    return NOMEM;
}

static unsigned ac_add_queue(ac_s *ac, unsigned int state, state_queue_s * queue)
{
    state_queue_node_s *node = NULL;
    
    if((NULL == queue) || (0 == state))
    {
        return INVALID;
    }
    
    node = (state_queue_node_s *)mem_new_rec(ac->mgr_state_queue_node);
    if(NULL == node)
    {
        return NOMEMl;
    }
    node->state = state;
    node->next = NULL;
    
    if(NULL == queue->head)
    {
        queue->head = node;
        queue->tail = node;
    }
    else
    {
        queue->tail->next = node;
        queue->tail = node;
    }
    
    return OK;
}

static state_queue_node_s *ac_del_queue(state_queue_s *queue)
{
    state_queue_node_s *node = NULL;
    
    if(NULL == queue)
    {
        return NULL;
    }
    
    if(NULL != queue->head)
    {
        node = queue->head;
        queue->head = queue->head-next;
        if(NULL == queue->head)
        {
            queue->tail = NULL;
        }
    }
    return node;
}

static void ac_free_queue(ac_s *ac)
{
    if(NULL != ac->mgr_state_queue_node)
    {
        mem_free_rec_mgr(ac->mgr_state_queue_node);
        ac->mgr_state_queue_node = NULL;
    }
    return;
}

static void ac_init_bitmap_popcount_table(ac_s *ac)
{
    unsigned short i, j;
    unsigned char count = 0;
    
    for(i = 0; i < AC_ALPHABET_SIZE; i++)
    {
        j = i;
        while(j)
        {
            if(j&1)
            {
                count ++;
            }
            j >>= 1;
        }
        ac->bitmap_popcount_talbe[i] = count;
        count = 0;
    }
    return ;
}

ac_s *ac_new(unsigned int method, unsigned int threshold, unsigned int res_len_flag)
{
    ac_s *ac = NULL;
    
    ac = (ac_s*)ac_malloc(sizeof(ac_s))
    if(NULL != ac)
    {
        memset(ac, 0, sizeof(ac_s));
        if(threshold > AC_THRESHOLD_DEFAULT)
        {
            ac->threshold_state = threshold;
        }
        else
        {
            ac->threshold_state = AC_THRESHOLD_DEFAULT;
        }
        ac->method = (snum ac_mem)method;
        ac->res_len_flag = res_len_flag;
        ac_init_bitmap_popcount_table(ac);
        return ac;
    }
    
    return NULL;
}

static void ac_free_pattern(ac_s *ac)
{
    ac_pattern_s *head = NULL;
    ac_pattern_s *ac_pattern = NULL;
    
    if(NULL = ac)
    {
        return ;
    }
    
    head = ac->head;
    while(NULL != head)
    {
        ac_patter = head;
        head = head->next;
        ac_free(ac_pattern->pattern);
    }
    ac->head = NULL;
    
    if(NULL != ac->mgr_ac_pattern)
    {
        mem_free_rec_mgr(ac->mgr_ac_pattern);
        ac->mgr_ac_pattern = NULL;
    }
    return;
}

static void ac_free_trans_table(ac_s* ac)
{
    if(NULL != ac->trans_table)
    {
        ac_free(ac->trans_table);
        ac->trans_table = NULL;
    }
    return ;
}

static void ac_free_match_table(ac_s *ac)
{
    if(NULL != ac->match_table)
    {
        ac_free(ac->match_table);
        ac->match_table = NULL;
    }
    return;
}

static void ac_free_fail_table(ac_s *ac)
{
    if(NULL != ac->fail_table)
    {
        ac_free(ac->fail_talbe);
        ac->fail_table = NULL;
    }
    return;
}

static void ac_free_bitmap_state_table(ac_s *ac)
{
    unsigned int **popcount_bitmap_states_table = ac->popcount_bitmap_states_table;
    unsigned int i = 0;
    unsigned int *popcount_bitmap_states = NULL;
    unsigned int num_states = (ac->num_states - ac->threshold_state);
    
    if(NULL != popcount_bitmap_states_table)
    {
        for(i = 0; i < num_states; i ++)
        {
            popcount_bitmap_states = popcount_bitmap_states_table[i];
            if(NULL != popcount_bitmap_states)
            {
                ac_free(popcount_bitmap_states);
                popcount_bitmap_states_table[i] = NULL;
            }
        }
        
        ac_free(popcount_bitmap_states_table);
        ac->popcount_bitmap_states_table = NULL;
    }
    return;
}

void ac_del(ac_s *ac)
{
    if(NULL = ac)
    {
        return;
    }
    
    ac_free_pattern(ac);
    
    ac_free_trans_table(ac);
    if(NULL != ac->mgr_ac_trans_node)
    {
        mem_free_rec_mgr(ac->mgr_ac_trans_node);
        ac->mgr_ac_trans_node = NULL;
    }
    
    ac_free_match_table(ac);
    if(NULL != ac->mgr_ac_id)
    {
        mem_free_rec_mgr(ac->mgr_ac_id);
        ac->mgr_ac_id = NULL;
    }
    
    ac_free_fail_table(ac);
    if(NULL != ac->full_state_table)
    {
        if(NULL != ac->full_state_table[0])
        {
            ac_free_(ac->full_state_table[0])
            ac->full_state_table[0] = NULL;
        }
        ac_free(ac_full_state_table);
        ac->full_state_table = NULL;
    }
    
    ac_free_bitmap_state_table(ac);
    
    ac_free(ac);
    ac = NULL; //bug
    
    return;
}

unsigned int ac_add_pattern(ac_s *ac, unsigned char pattern, unsigned int len, unsigned int id)
{
    ac_pattern_s *ac_pattern = NULL;
    
    if(NULL == ac || NULL == pattern) || 0 == len)
    {
        return INVALID;
    }
    
    if(NULL == ac->mgr_ac_pattern)
    {
        ac->mgr_ac_pattern = mem_new_rec_mgr(sizeof(ac_pattern_s));
        if(NULL == ac->mgr_ac_pattern)
        {
            goto add_pattern_err;
        }
    }
    
    ac_pattern = (ac_pattern_s *)mem_new_rec(ac->mgr_ac_pattern)
    if(NULL == ac_pattern)
    {
        goto add_pattern_err;
    }
    ac_pattern->pattern = (unsigned char *)ac_malloc(sizeof(unsigned char) * len);
    if(NULL == ac_pattern->pattern)
    {
        goto add_pattern_err;
    }
    memcpy(ac_pattern->pattern, pattern, len);
    ac_pattern->len = len;
    ac_pattern->id = id;
    ac_pattern->next = ac->head;
    ac->head = ac_pattern;
    
    return OK;
    
add_pattern_err:
    if(NULL != ac_pattern)
    {
        ac_free(ac_pattern)
    }
    return NOMEM;
}

static ac_id_s *ac_copy_id(ac_s *ac, ac_id_s *src_id);
{
    ac_id_s *dst_id = NULL;
    
    dst_id = (ac_id_s *)mem_new_rec(ac->mgr_ac_id)
    if(NULL == dst_id)
    {
        return NULL;
    }
    memcpy(dst_id, src_id, sizeof(ac_id_s));
    return dst_id;
}

static unsigned int ac_get_next_state(ac_s *ac, unsigned int state, unsigned char input)
{
    ac_trans_node_s *node = NULL;
    
    for(node= ac->trans_table[state].head; node != NULL ; node = node->next)
    {
        if(AC_INPUT(node->next_state) == input)
        {
            return AC_STATE(node->next_state);
        }
    }
    
    if(0 == state)
    {
        return 0;
    }
    else
    {
        return AC_FAIL_STATE;
    }
}

static unsigned int ac_put_next_state(ac_s *ac, ac_trans_s *trans_table, unsigned char input, unsigned int next_state)
{
    ac_trans_node_s *node = NULL;
    
    node = (ac_trans_node_s *)mem_new_rec(ac->mgr_ac_trans_node);
    if(NULL == node)
    {
        return NOMEM;
    }
    node->next_state = ((input<<AC_VALUT_SHIFT)|next_state);
    node->next = trans_table->head;
    trans_table->head = node;
    return OK;
}

static unsigned int ac_add_pattern_state(ac_s *ac, ac_pattern_s *ac_pattern)
{
    unsigned char *pattern = NULL;
    unsigned char t_nocase ;
    unsigned int pattern_len = 0;
    unsigned int state = 0;
    unsigned int next_state = 0;
    unsigned short res_len = 0;
    ac_id_s *id = NULL;
    
    pattern = ac_pattern->pattern;
    pattern_len = ac_pattern->len;
    for(; pattern_len > 0; pattern ++, pattern_len--)
    {
        t_nocase = *pattern;
        if((t_nocase >= 'a') && (t_nocase <= 'z'))
        {
            t_nocase = (unsigned char )((t_nocase-'z')+'A');
        }
        
        next_state = ac_get_next_state(ac, state, t_nocase);
        if((0 == next_state) || AC_FAIL_STATE == next_state)
        {
            break;
        }
        state = next_state;
        res_len ++;
    }
    
    for(; pattern_len > 0; pattern++, pattern_len--)
    {
        ac->num_states ++;
        res_len ++;
        
        t_nocase = *pattern;
        if((t_nocase >= 'a') && (t_nocase <= 'z'))
        {
            t_nocase = (unsigned char)((t_nocase-'a')+'A');
        }
        
        if(OK != ac_put_next_state(ac, &(ac->trans_table[state]), t_nocase, ac->num_states))
        {
            return AC_NOMEM;
        }
        state = ac->num_states;
        ac->match_table[state].res_len = res_len;
    }
    
    id = (ac_is_s *)mem_new_rec(ac->mgr_ac_id);
    if(NULL == id)
    {
        return NOMEM;
    }
    
    if(NULL == ac->match_table[state].head)
    {
        ac->final_state_num ++;
    }
    
    id->id = ac_pattern->id;
    id->len = ac_pattern->len;
    id->next = ac->match_table[state].head;
    ac->match_table[state].head = id;
    
    return OK;
}

static unsigned ac_renumber(ac_s *ac, unsigned int *new_states)
{
    unsigned int ret = OK;
    state_queue_s state_queue;
    state_queue_s *queue = NULL;
    state_queue_node_s *queue_node = NULL;
    ac_trans_node_s *node = NULL;
    unsigned int del_state;
    unsigned i = 1;
    unsigned j = 0;
    
    ac_trans_s *trans_table = ac->trans_table;
    ac_match_s *match_table = ac->matach_table;
    queue = &state_queue;
    ret = ac_init_queue(ac, queue);
    if(OK != ret)
    {
        goto err_label;
    }
    
    for(node = trans_table[0].head; node != NULL; node = node->next)
    {
        ret = ac_add_queue(ac, AC_STATE(node->next_state), queue);
        if(OK != ret)
        {
            goto err_label;
        }
    }
    
    while(NULL != queue->head)
    {
        queue_node = ac_del_queue(queue);
        del_state = queue_node->state;
        mem_free_rec(ac->mgr_state_queue_node, queue_node);
        for(node = trans_table[del_state].head; node != NULL; node = node->next)
        {
            ret = ac_add_queue(ac, AC_STATE(node->next_state), queue);
            if(NULL != ret)
            {
                goto err_label;
            }
        }
        if(NULL == match_table[del_state].head)
        {
            new_states[del_state] = i;
            i ++;
        }
        else
        {
            new_state[del_state] = ac->min_final_state + j;
            j ++;
        }
    }
err_label:
    ac_free_queue(ac);
    return ret;
}

static void ac_put_renumber_state(ac_trans_s *trans_table, ac_trans_node_s *node)
{
    node->next = trans_table->head;
    trans_table->head = node;
    
    return;
}

static unsigned int ac_renumber_states(ac_s *ac)
{
    unsigned int ret = OK;
    unsigned int size;
    unsigned int *new_states = NULL;
    ac_trans_node_s *node = NULL, *temp_node = NULL;
    unsigned int i;
    unsigned int new_state;
    unsigned int num_states;
    ac_trans_s *trans_table = NULL;
    ac_match_s *match_table = NULL;
    unsigned int *fail_states = NULL;
    ac_trans_s *new_trans_table = NULL;
    ac_match_s *new_match_talbe = NULL;
    unsigned int *new_fail_states = NULL;

    num_states = ac->num_states;
    trans_table = ac->trans_table;
    match_table = ac->match_table;
    fail_states = ac->fail_states;

    size = sizeof(unsigned int)*num_states;
    new_states = ac_malloc(size);
    if(NULL == new_states)
    {
        goto err_label;
    }
    memset(new_states, 0, size);

    size = sizeof(ac_trans_s)*num_states;
    new_trans_table = ac_malloc(size);
    if(NULL == new_trans_talbe)
    {
        goto err_label;
    }
    memset(new_trans_talbe);

    if(RES_LEN_NEEDED == ac->res_len_flag)
    {
        size = sizeof(ac_match_s) * num_states;
    }
    else
    {
        size = sizeof(ac_match_s) * ac->final_state_num;
    }
    new_match_table = ac_malloc(size);
    if(NULL == new_match_table)
    {
        goto err_label;
    }
    memset(new_match_table, 0, size);
    
    ac->min_final_state = ac->num_states - ac->final_state_num;
    size = sizeof(unsigned int) * num_states;
    new_fail_states = ac_malloc(size);
    if(NULL == new_fail_states)
    {
        goto err_label;
    }
    memset(new_fail_states, 0, size);
    
    ret = ac_renumber(ac, new_states);
    if(OK != ret)
    {
        goto err_label;
    }
    
    /*调整状态*/
    for(i = 0; i < num_states; i ++)
    {
        for(node = trans_table[i].head; node != NULL; node = temp_node) 
        {
            temp_ndoe = node->next;
            node->next_state = ((node->next_state & ~AC_MAX_STATE) | new_states[AC_STATE(node->next_state)]);
            ac_put_renumber_state(&new_trans_table[new_states[i]], node);
            node = NULL;
        }
        
        trans_table[i].head = NULL;
        new_state = new_states[i];

        if(RES_LEN_NEEDED == ac->res_len_flag)
        {
            new_match_table[new_state].head = match_table[i].head;
            new_match_table[new_state].res_len = match_table[i].res_len;
        }
        else if(NULL != match_table[i].head)
        {
            new_match_table[new_state - ac->min_final_state].head = match_table[i].head;
        }
        match_table[i].head = NULL;
        
        new_fail_states[new_state] = new_states[fail_states[i]];
    }    
    
    ac_free_trans_table(ac);
    ac->trans_table = new_trans_table;
    new_trans_table = NULL;
    
    ac_free_match_table(ac);
    ac->match_table = new_match_table;
    new_match_table = NULL;

    ac_free_fail_state(ac);
    ac->fail_states = new_fail_states;
    new_fail_states = NULL;

    ac_free(new_states);
    new_states = NULL;

    return OK;

err_label:
    if(NULL != new_states)
    {
        ac_free(new_states);
    }
    if(NULL != new_trans_table)
    {
        ac_free(new_trans_table);
    }
    if(NULL != new_match_table);
    {
        ac_free(new_match_table);
    }
    if(NULL != new_fail_states)
    {
        ac_free(new_fail_states);
    }
    return NOMEM;
}


