#ifndef COMMON_H
#define COMMON_H

/*
 *      指令转换文件
 */

#define IS_BASE_INSTRUCT(x)     ((x>INSTRUCT_BASE)&&(x<INSTRUCT_BASEEND))?1:0
#define IS_ADVANCE_INSTRUCT(x)  ((x>INSTRUCT_ADVANCE)&&(x<INSTRUCT_ADVANCEEND))?1:0
#define IS_ALIGN_INSTRUCT(x)    ((x>=INSTRUCT_ALIGN_HEAD)&&(x<=INSTRUCT_ALIGN_TAIL))?1:0

#define INSTRUCT_ALIGN_HEAD     1
#define INSTRUCT_ALIGN_MID      2
#define INSTRUCT_ALIGN_TAIL     3

#define INSTRUCT_BASE           1000
#define INSTRUCT_BASE_STA       1001
#define INSTRUCT_BASE_END       1002
#define INSTRUCT_BASE_DEY       1003
#define INSTRUCT_BASE_MOV       1004
#define INSTRUCT_BASE_CMG       1005
#define INSTRUCT_BASEEND        1999

#define INSTRUCT_ADVANCE        2000        //都需要输入各载体车当前位置，rfid数量
#define INSTRUCT_ADVANCE_PUD    2001        //Pathway Uniform Distribution 轨道均布
#define INSTRUCT_ADVANCE_SOD    2002        //Space Out Distribution 间隔分布(int 间距，对齐车[头\中\尾])
#define INSTRUCT_ADVANCE_GAT    2003        //Gather 集合 (int 对齐车[头\中\尾])
#define INSTRUCT_ADVANCE_EXP    2004        //Expand 膨胀 (int 倍率,int 对齐车[头\中\尾])
#define INSTRUCT_ADVANCE_KDM    2005        //Keep Distance Move 保持当前间距移动(int 向轨道[头\中\尾]运动)
#define INSTRUCT_ADVANCEEND     2999

#endif // COMMON_H
