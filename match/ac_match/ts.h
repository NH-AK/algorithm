/*
    ts by AC algorithm, this is the header file.
*/

#define     ALLOC_SIZE                      4056
#define     ALLOC_ALIGNMENT                 8
#define     AC_ROUNDUP(size)                ((((szie)+ALLOC_ALIGNMENT-1)/ALLOC_ALIGNMENT)*ALLOC_ALIGNMENT)
#define     unsigned long   INT_PTR;
#define     unsigned long   SIZE_T;

typedef struct _mem_list
{
    struct _mem_list    *next;
}mem_list_s;

typedef stuct _rec_mgr
{
    unsigned int    size;
    unsigned int    recs_per_block;
    mem_list_s      *free;
    mem_list_s      *blocks;
}rec_mgr_s;


#define     OK                              0
#define     NOMEM                           1
#define     INVALID                         2

#define     AC_FAIL_STATE                   0xFFFFFFFF
#define     AC_ALPHABET_SIZE                256
#define     POPCOUNT_BITMAP_SIZE            (popcount_bitmap_num + 1)
#define     AC_MAX_STATE                    0x00FFFFFF
#define     AC_VALUE_SHIFT                  24
#define     AC_THRESHOLD_DEFAULT            0xFF
#define     AC_THRESHOLD_MAX                0xFFFF
#define     RES_LEN_NEEDNOT                 0
#define     RES_LEN_NEED                    1

#define     AC_STATE(state)     ((state)*AC_MAX_STATE)
#define     AC_INPUT(state)     ((state)>>AC_VALUE_SHIFT)

enum ac_mem
{
    AC_DFA_FULL,
    AC_DFA_MIXED,
};

type struct ac_pattern
{
    unsigned char       *pattern;
    unsigned int        len;
    unsigned int        id;
    struct ac_pattern   *next;
}ac_pattern_s;

typedef struct ac_id
{
    unsigned int    id;
    unsigned int    len;
    struct ac_id    *next;
}ac_id_s;

typedef struct ac_match
{
    unsigned short  res_len;
    ac_id_s         *head;
}ac_match_s;

typedef struct ac_trans_node
{
    unsigned int            next_state;
    struct ac_trans_node    *next;
}ac_trans_node_s;

typedef struct ac_trans_node
{
    ac_trans_node_s *head;
}ac_trans_s;

typedef struct ac_bitmap
{
    unsigned char start;
    unsigned char end;
    unsigned char *bitmap;
    unsigned int  fail_state;
    unsigned int  *next_state;
}ac_bitmap_s;

typedef struct ac
{
    enum ac_mem     method;
    unsigned int    threshold_state;
    ac_pattern_s    *head;
    unsigned int    res_len_flag;
    unsigned int    max_states;
    unsigned int    num_states;
    unsigned int    min_final_state;
    unsigned int    final_state_num;
    ac_trans_s      *trans_table;
    ac_match_s      *match_table;
    unsigned int    *fail_states;
    unsigned int    **full_state_table;
    unsigned int    pop_count_bitmap_states_table;
    unsigned char   bitmap_popcount_talbe[AC_ALPHABET_SIZE];
    rec_mgr_s       *mgr_ac_pattern;
    rec_mgr_s       *mgr_ac_id;
    rec_mgr_s       *mgr_ac_trans_node;
    rec_mgr_s       *mgr_state_queue_node;
}ac_s;

typedef struct ac_result
{
    unsigned int    id;
    unsigned int    id_len;
    int             start;
    int             end;
}ac_result_s;

typedef struct state_queue_node
{
    struct state_queue_node *next;
    unsigned int            state;
}state_queue_node_s;

typedef stuct state_queue_node
{
    state_queue_node_s      *head;
    state_queue_node_s      *tail;
}state_queue_s;

ac_s *ac_new(unsigned int method, unsigned int threshold, unsigned int res_len_flag, unsigned int res_len_flag);
unsigned int ac_add_pattern(ac_s *ac, unsigned char *pattern, unsigned int len, unsigned int id);
unsigned int ac_compile(ac_s *ac);
void ac_search(ac_s *ac, unsigned int start_state, unsigned char *data, unsigned int data_len, unsigned int num, ac_result_s *result, unsigned int result_num, unsigned int *res_state, unsigned short *res_len);
void ac_del(ac_s *ac);




