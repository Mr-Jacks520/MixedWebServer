#ifndef _ERRS_H
#define _ERRS_H

/**
 * @brief 错误判断
 * 
 * @param cond 条件
 * @param msg 条件满足时的提示信息
 */
void errif(bool cond, const char* msg);

#endif // _ERRS_H 