/*
 * IDENTIFICATION:
 * stub generated Sat Jan  8 19:30:45 2011
 * with a MiG generated Mon May 18 09:59:33 PDT 2009 by root@sulitlana.apple.com
 * OPTIONS: 
 */
#define	__MIG_check__Reply__mach_host_subsystem__ 1
#define	__NDR_convert__Reply__mach_host_subsystem__ 1
#define	__NDR_convert__mig_reply_error_subsystem__ 1

#include "mach_host.h"


#ifndef	mig_internal
#define	mig_internal	static __inline__
#endif	/* mig_internal */

#ifndef	mig_external
#define mig_external
#endif	/* mig_external */

#if	!defined(__MigTypeCheck) && defined(TypeCheck)
#define	__MigTypeCheck		TypeCheck	/* Legacy setting */
#endif	/* !defined(__MigTypeCheck) */

#if	!defined(__MigKernelSpecificCode) && defined(_MIG_KERNEL_SPECIFIC_CODE_)
#define	__MigKernelSpecificCode	_MIG_KERNEL_SPECIFIC_CODE_	/* Legacy setting */
#endif	/* !defined(__MigKernelSpecificCode) */

#ifndef	LimitCheck
#define	LimitCheck 0
#endif	/* LimitCheck */

#ifndef	min
#define	min(a,b)  ( ((a) < (b))? (a): (b) )
#endif	/* min */

#if !defined(_WALIGN_)
#define _WALIGN_(x) (((x) + 3) & ~3)
#endif /* !defined(_WALIGN_) */

#if !defined(_WALIGNSZ_)
#define _WALIGNSZ_(x) _WALIGN_(sizeof(x))
#endif /* !defined(_WALIGNSZ_) */

#ifndef	UseStaticTemplates
#define	UseStaticTemplates	0
#endif	/* UseStaticTemplates */

#ifndef	__MachMsgErrorWithTimeout
#define	__MachMsgErrorWithTimeout(_R_) { \
	switch (_R_) { \
	case MACH_SEND_INVALID_DATA: \
	case MACH_SEND_INVALID_DEST: \
	case MACH_SEND_INVALID_HEADER: \
		mig_put_reply_port(InP->Head.msgh_reply_port); \
		break; \
	case MACH_SEND_TIMED_OUT: \
	case MACH_RCV_TIMED_OUT: \
	default: \
		mig_dealloc_reply_port(InP->Head.msgh_reply_port); \
	} \
}
#endif	/* __MachMsgErrorWithTimeout */

#ifndef	__MachMsgErrorWithoutTimeout
#define	__MachMsgErrorWithoutTimeout(_R_) { \
	switch (_R_) { \
	case MACH_SEND_INVALID_DATA: \
	case MACH_SEND_INVALID_DEST: \
	case MACH_SEND_INVALID_HEADER: \
		mig_put_reply_port(InP->Head.msgh_reply_port); \
		break; \
	default: \
		mig_dealloc_reply_port(InP->Head.msgh_reply_port); \
	} \
}
#endif	/* __MachMsgErrorWithoutTimeout */

#ifndef	__DeclareSendRpc
#define	__DeclareSendRpc(_NUM_, _NAME_)
#endif	/* __DeclareSendRpc */

#ifndef	__BeforeSendRpc
#define	__BeforeSendRpc(_NUM_, _NAME_)
#endif	/* __BeforeSendRpc */

#ifndef	__AfterSendRpc
#define	__AfterSendRpc(_NUM_, _NAME_)
#endif	/* __AfterSendRpc */

#ifndef	__DeclareSendSimple
#define	__DeclareSendSimple(_NUM_, _NAME_)
#endif	/* __DeclareSendSimple */

#ifndef	__BeforeSendSimple
#define	__BeforeSendSimple(_NUM_, _NAME_)
#endif	/* __BeforeSendSimple */

#ifndef	__AfterSendSimple
#define	__AfterSendSimple(_NUM_, _NAME_)
#endif	/* __AfterSendSimple */

#define msgh_request_port	msgh_remote_port
#define msgh_reply_port		msgh_local_port



#if ( __MigTypeCheck || __NDR_convert__ )
#if __MIG_check__Reply__mach_host_subsystem__
#if !defined(__MIG_check__Reply__host_info_t__defined)
#define __MIG_check__Reply__host_info_t__defined
#ifndef __NDR_convert__int_rep__Reply__host_info_t__RetCode__defined
#if	defined(__NDR_convert__int_rep__mach_host__kern_return_t__defined)
#define	__NDR_convert__int_rep__Reply__host_info_t__RetCode__defined
#define	__NDR_convert__int_rep__Reply__host_info_t__RetCode(a, f) \
	__NDR_convert__int_rep__mach_host__kern_return_t((kern_return_t *)(a), f)
#elif	defined(__NDR_convert__int_rep__kern_return_t__defined)
#define	__NDR_convert__int_rep__Reply__host_info_t__RetCode__defined
#define	__NDR_convert__int_rep__Reply__host_info_t__RetCode(a, f) \
	__NDR_convert__int_rep__kern_return_t((kern_return_t *)(a), f)
#endif /* defined(__NDR_convert__*__defined) */
#endif /* __NDR_convert__int_rep__Reply__host_info_t__RetCode__defined */


#ifndef __NDR_convert__int_rep__Reply__host_info_t__host_info_out__defined
#if	defined(__NDR_convert__int_rep__mach_host__host_info_t__defined)
#define	__NDR_convert__int_rep__Reply__host_info_t__host_info_out__defined
#define	__NDR_convert__int_rep__Reply__host_info_t__host_info_out(a, f, c) \
	__NDR_convert__int_rep__mach_host__host_info_t((host_info_t *)(a), f, c)
#elif	defined(__NDR_convert__int_rep__host_info_t__defined)
#define	__NDR_convert__int_rep__Reply__host_info_t__host_info_out__defined
#define	__NDR_convert__int_rep__Reply__host_info_t__host_info_out(a, f, c) \
	__NDR_convert__int_rep__host_info_t((host_info_t *)(a), f, c)
#elif	defined(__NDR_convert__int_rep__mach_host__integer_t__defined)
#define	__NDR_convert__int_rep__Reply__host_info_t__host_info_out__defined
#define	__NDR_convert__int_rep__Reply__host_info_t__host_info_out(a, f, c) \
	__NDR_convert__ARRAY((integer_t *)(a), f, c, __NDR_convert__int_rep__mach_host__integer_t)
#elif	defined(__NDR_convert__int_rep__integer_t__defined)
#define	__NDR_convert__int_rep__Reply__host_info_t__host_info_out__defined
#define	__NDR_convert__int_rep__Reply__host_info_t__host_info_out(a, f, c) \
	__NDR_convert__ARRAY((integer_t *)(a), f, c, __NDR_convert__int_rep__integer_t)
#elif	defined(__NDR_convert__int_rep__mach_host__int32_t__defined)
#define	__NDR_convert__int_rep__Reply__host_info_t__host_info_out__defined
#define	__NDR_convert__int_rep__Reply__host_info_t__host_info_out(a, f, c) \
	__NDR_convert__ARRAY((int32_t *)(a), f, c, __NDR_convert__int_rep__mach_host__int32_t)
#elif	defined(__NDR_convert__int_rep__int32_t__defined)
#define	__NDR_convert__int_rep__Reply__host_info_t__host_info_out__defined
#define	__NDR_convert__int_rep__Reply__host_info_t__host_info_out(a, f, c) \
	__NDR_convert__ARRAY((int32_t *)(a), f, c, __NDR_convert__int_rep__int32_t)
#endif /* defined(__NDR_convert__*__defined) */
#endif /* __NDR_convert__int_rep__Reply__host_info_t__host_info_out__defined */


#ifndef __NDR_convert__int_rep__Reply__host_info_t__host_info_outCnt__defined
#if	defined(__NDR_convert__int_rep__mach_host__mach_msg_type_number_t__defined)
#define	__NDR_convert__int_rep__Reply__host_info_t__host_info_outCnt__defined
#define	__NDR_convert__int_rep__Reply__host_info_t__host_info_outCnt(a, f) \
	__NDR_convert__int_rep__mach_host__mach_msg_type_number_t((mach_msg_type_number_t *)(a), f)
#elif	defined(__NDR_convert__int_rep__mach_msg_type_number_t__defined)
#define	__NDR_convert__int_rep__Reply__host_info_t__host_info_outCnt__defined
#define	__NDR_convert__int_rep__Reply__host_info_t__host_info_outCnt(a, f) \
	__NDR_convert__int_rep__mach_msg_type_number_t((mach_msg_type_number_t *)(a), f)
#endif /* defined(__NDR_convert__*__defined) */
#endif /* __NDR_convert__int_rep__Reply__host_info_t__host_info_outCnt__defined */



#ifndef __NDR_convert__char_rep__Reply__host_info_t__host_info_out__defined
#if	defined(__NDR_convert__char_rep__mach_host__host_info_t__defined)
#define	__NDR_convert__char_rep__Reply__host_info_t__host_info_out__defined
#define	__NDR_convert__char_rep__Reply__host_info_t__host_info_out(a, f, c) \
	__NDR_convert__char_rep__mach_host__host_info_t((host_info_t *)(a), f, c)
#elif	defined(__NDR_convert__char_rep__host_info_t__defined)
#define	__NDR_convert__char_rep__Reply__host_info_t__host_info_out__defined
#define	__NDR_convert__char_rep__Reply__host_info_t__host_info_out(a, f, c) \
	__NDR_convert__char_rep__host_info_t((host_info_t *)(a), f, c)
#elif	defined(__NDR_convert__char_rep__mach_host__integer_t__defined)
#define	__NDR_convert__char_rep__Reply__host_info_t__host_info_out__defined
#define	__NDR_convert__char_rep__Reply__host_info_t__host_info_out(a, f, c) \
	__NDR_convert__ARRAY((integer_t *)(a), f, c, __NDR_convert__char_rep__mach_host__integer_t)
#elif	defined(__NDR_convert__char_rep__integer_t__defined)
#define	__NDR_convert__char_rep__Reply__host_info_t__host_info_out__defined
#define	__NDR_convert__char_rep__Reply__host_info_t__host_info_out(a, f, c) \
	__NDR_convert__ARRAY((integer_t *)(a), f, c, __NDR_convert__char_rep__integer_t)
#elif	defined(__NDR_convert__char_rep__mach_host__int32_t__defined)
#define	__NDR_convert__char_rep__Reply__host_info_t__host_info_out__defined
#define	__NDR_convert__char_rep__Reply__host_info_t__host_info_out(a, f, c) \
	__NDR_convert__ARRAY((int32_t *)(a), f, c, __NDR_convert__char_rep__mach_host__int32_t)
#elif	defined(__NDR_convert__char_rep__int32_t__defined)
#define	__NDR_convert__char_rep__Reply__host_info_t__host_info_out__defined
#define	__NDR_convert__char_rep__Reply__host_info_t__host_info_out(a, f, c) \
	__NDR_convert__ARRAY((int32_t *)(a), f, c, __NDR_convert__char_rep__int32_t)
#endif /* defined(__NDR_convert__*__defined) */
#endif /* __NDR_convert__char_rep__Reply__host_info_t__host_info_out__defined */




#ifndef __NDR_convert__float_rep__Reply__host_info_t__host_info_out__defined
#if	defined(__NDR_convert__float_rep__mach_host__host_info_t__defined)
#define	__NDR_convert__float_rep__Reply__host_info_t__host_info_out__defined
#define	__NDR_convert__float_rep__Reply__host_info_t__host_info_out(a, f, c) \
	__NDR_convert__float_rep__mach_host__host_info_t((host_info_t *)(a), f, c)
#elif	defined(__NDR_convert__float_rep__host_info_t__defined)
#define	__NDR_convert__float_rep__Reply__host_info_t__host_info_out__defined
#define	__NDR_convert__float_rep__Reply__host_info_t__host_info_out(a, f, c) \
	__NDR_convert__float_rep__host_info_t((host_info_t *)(a), f, c)
#elif	defined(__NDR_convert__float_rep__mach_host__integer_t__defined)
#define	__NDR_convert__float_rep__Reply__host_info_t__host_info_out__defined
#define	__NDR_convert__float_rep__Reply__host_info_t__host_info_out(a, f, c) \
	__NDR_convert__ARRAY((integer_t *)(a), f, c, __NDR_convert__float_rep__mach_host__integer_t)
#elif	defined(__NDR_convert__float_rep__integer_t__defined)
#define	__NDR_convert__float_rep__Reply__host_info_t__host_info_out__defined
#define	__NDR_convert__float_rep__Reply__host_info_t__host_info_out(a, f, c) \
	__NDR_convert__ARRAY((integer_t *)(a), f, c, __NDR_convert__float_rep__integer_t)
#elif	defined(__NDR_convert__float_rep__mach_host__int32_t__defined)
#define	__NDR_convert__float_rep__Reply__host_info_t__host_info_out__defined
#define	__NDR_convert__float_rep__Reply__host_info_t__host_info_out(a, f, c) \
	__NDR_convert__ARRAY((int32_t *)(a), f, c, __NDR_convert__float_rep__mach_host__int32_t)
#elif	defined(__NDR_convert__float_rep__int32_t__defined)
#define	__NDR_convert__float_rep__Reply__host_info_t__host_info_out__defined
#define	__NDR_convert__float_rep__Reply__host_info_t__host_info_out(a, f, c) \
	__NDR_convert__ARRAY((int32_t *)(a), f, c, __NDR_convert__float_rep__int32_t)
#endif /* defined(__NDR_convert__*__defined) */
#endif /* __NDR_convert__float_rep__Reply__host_info_t__host_info_out__defined */




mig_internal kern_return_t __MIG_check__Reply__host_info_t(__Reply__host_info_t *Out0P)
{

	typedef __Reply__host_info_t __Reply;
#if	__MigTypeCheck
	unsigned int msgh_size;
#endif	/* __MigTypeCheck */

	if (Out0P->Head.msgh_id != 300) {
	    if (Out0P->Head.msgh_id == MACH_NOTIFY_SEND_ONCE)
		{ return MIG_SERVER_DIED; }
	    else
		{ return MIG_REPLY_MISMATCH; }
	}

#if	__MigTypeCheck
	msgh_size = Out0P->Head.msgh_size;

	if ((Out0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) ||
	    ((msgh_size > (mach_msg_size_t)sizeof(__Reply) || msgh_size < (mach_msg_size_t)(sizeof(__Reply) - 60)) &&
	     (msgh_size != (mach_msg_size_t)sizeof(mig_reply_error_t) ||
	      Out0P->RetCode == KERN_SUCCESS)))
		{ return MIG_TYPE_ERROR ; }
#endif	/* __MigTypeCheck */

	if (Out0P->RetCode != KERN_SUCCESS) {
#ifdef	__NDR_convert__mig_reply_error_t__defined
		__NDR_convert__mig_reply_error_t((mig_reply_error_t *)Out0P);
#endif	/* __NDR_convert__mig_reply_error_t__defined */
		return ((mig_reply_error_t *)Out0P)->RetCode;
	}

#if defined(__NDR_convert__int_rep__Reply__host_info_t__host_info_outCnt__defined)
	if (Out0P->NDR.int_rep != NDR_record.int_rep)
		__NDR_convert__int_rep__Reply__host_info_t__host_info_outCnt(&Out0P->host_info_outCnt, Out0P->NDR.int_rep);
#endif	/* __NDR_convert__int_rep__Reply__host_info_t__host_info_outCnt__defined */
#if	__MigTypeCheck
	if (msgh_size != (mach_msg_size_t)(sizeof(__Reply) - 60) + ((4 * Out0P->host_info_outCnt)))
		{ return MIG_TYPE_ERROR ; }
#endif	/* __MigTypeCheck */

#if	defined(__NDR_convert__int_rep__Reply__host_info_t__RetCode__defined) || \
	defined(__NDR_convert__int_rep__Reply__host_info_t__host_info_out__defined) || \
	defined(__NDR_convert__int_rep__Reply__host_info_t__host_info_outCnt__defined)
	if (Out0P->NDR.int_rep != NDR_record.int_rep) {
#if defined(__NDR_convert__int_rep__Reply__host_info_t__RetCode__defined)
		__NDR_convert__int_rep__Reply__host_info_t__RetCode(&Out0P->RetCode, Out0P->NDR.int_rep);
#endif /* __NDR_convert__int_rep__Reply__host_info_t__RetCode__defined */
#if defined(__NDR_convert__int_rep__Reply__host_info_t__host_info_out__defined)
		__NDR_convert__int_rep__Reply__host_info_t__host_info_out(&Out0P->host_info_out, Out0P->NDR.int_rep, Out0P->host_info_outCnt);
#endif /* __NDR_convert__int_rep__Reply__host_info_t__host_info_out__defined */
	}
#endif	/* defined(__NDR_convert__int_rep...) */

#if	0 || \
	defined(__NDR_convert__char_rep__Reply__host_info_t__host_info_out__defined) || \
	0
	if (Out0P->NDR.char_rep != NDR_record.char_rep) {
#if defined(__NDR_convert__char_rep__Reply__host_info_t__host_info_out__defined)
		__NDR_convert__char_rep__Reply__host_info_t__host_info_out(&Out0P->host_info_out, Out0P->NDR.char_rep, Out0P->host_info_outCnt);
#endif /* __NDR_convert__char_rep__Reply__host_info_t__host_info_out__defined */
	}
#endif	/* defined(__NDR_convert__char_rep...) */

#if	0 || \
	defined(__NDR_convert__float_rep__Reply__host_info_t__host_info_out__defined) || \
	0
	if (Out0P->NDR.float_rep != NDR_record.float_rep) {
#if defined(__NDR_convert__float_rep__Reply__host_info_t__host_info_out__defined)
		__NDR_convert__float_rep__Reply__host_info_t__host_info_out(&Out0P->host_info_out, Out0P->NDR.float_rep, Out0P->host_info_outCnt);
#endif /* __NDR_convert__float_rep__Reply__host_info_t__host_info_out__defined */
	}
#endif	/* defined(__NDR_convert__float_rep...) */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Reply__host_info_t__defined) */
#endif /* __MIG_check__Reply__mach_host_subsystem__ */
#endif /* ( __MigTypeCheck || __NDR_convert__ ) */


/* Routine host_info */
mig_external kern_return_t host_info
(
	host_t host,
	host_flavor_t flavor,
	host_info_t host_info_out,
	mach_msg_type_number_t *host_info_outCnt
)
{

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		NDR_record_t NDR;
		host_flavor_t flavor;
		mach_msg_type_number_t host_info_outCnt;
	} Request;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		NDR_record_t NDR;
		kern_return_t RetCode;
		mach_msg_type_number_t host_info_outCnt;
		integer_t host_info_out[15];
		mach_msg_trailer_t trailer;
	} Reply;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		NDR_record_t NDR;
		kern_return_t RetCode;
		mach_msg_type_number_t host_info_outCnt;
		integer_t host_info_out[15];
	} __Reply;
#ifdef  __MigPackStructs
#pragma pack()
#endif
	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	union {
		Request In;
		Reply Out;
	} Mess;

	Request *InP = &Mess.In;
	Reply *Out0P = &Mess.Out;

	mach_msg_return_t msg_result;

#ifdef	__MIG_check__Reply__host_info_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Reply__host_info_t__defined */

	__DeclareSendRpc(200, "host_info")

	InP->NDR = NDR_record;

	InP->flavor = flavor;

	if (*host_info_outCnt < 15)
		InP->host_info_outCnt = *host_info_outCnt;
	else
		InP->host_info_outCnt = 15;

	InP->Head.msgh_bits =
		MACH_MSGH_BITS(19, MACH_MSG_TYPE_MAKE_SEND_ONCE);
	/* msgh_size passed as argument */
	InP->Head.msgh_request_port = host;
	InP->Head.msgh_reply_port = mig_get_reply_port();
	InP->Head.msgh_id = 200;

	__BeforeSendRpc(200, "host_info")
	msg_result = mach_msg(&InP->Head, MACH_SEND_MSG|MACH_RCV_MSG|MACH_MSG_OPTION_NONE, (mach_msg_size_t)sizeof(Request), (mach_msg_size_t)sizeof(Reply), InP->Head.msgh_reply_port, MACH_MSG_TIMEOUT_NONE, MACH_PORT_NULL);
	__AfterSendRpc(200, "host_info")
	if (msg_result != MACH_MSG_SUCCESS) {
		__MachMsgErrorWithoutTimeout(msg_result);
		{ return msg_result; }
	}


#if	defined(__MIG_check__Reply__host_info_t__defined)
	check_result = __MIG_check__Reply__host_info_t((__Reply__host_info_t *)Out0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ return check_result; }
#endif	/* defined(__MIG_check__Reply__host_info_t__defined) */

	if (Out0P->host_info_outCnt > *host_info_outCnt) {
		(void)memcpy((char *) host_info_out, (const char *) Out0P->host_info_out, 4 *  *host_info_outCnt);
		*host_info_outCnt = Out0P->host_info_outCnt;
		{ return MIG_ARRAY_TOO_LARGE; }
	}
	(void)memcpy((char *) host_info_out, (const char *) Out0P->host_info_out, 4 * Out0P->host_info_outCnt);

	*host_info_outCnt = Out0P->host_info_outCnt;

	return KERN_SUCCESS;
}

#if ( __MigTypeCheck || __NDR_convert__ )
#if __MIG_check__Reply__mach_host_subsystem__
#if !defined(__MIG_check__Reply__host_kernel_version_t__defined)
#define __MIG_check__Reply__host_kernel_version_t__defined
#ifndef __NDR_convert__int_rep__Reply__host_kernel_version_t__RetCode__defined
#if	defined(__NDR_convert__int_rep__mach_host__kern_return_t__defined)
#define	__NDR_convert__int_rep__Reply__host_kernel_version_t__RetCode__defined
#define	__NDR_convert__int_rep__Reply__host_kernel_version_t__RetCode(a, f) \
	__NDR_convert__int_rep__mach_host__kern_return_t((kern_return_t *)(a), f)
#elif	defined(__NDR_convert__int_rep__kern_return_t__defined)
#define	__NDR_convert__int_rep__Reply__host_kernel_version_t__RetCode__defined
#define	__NDR_convert__int_rep__Reply__host_kernel_version_t__RetCode(a, f) \
	__NDR_convert__int_rep__kern_return_t((kern_return_t *)(a), f)
#endif /* defined(__NDR_convert__*__defined) */
#endif /* __NDR_convert__int_rep__Reply__host_kernel_version_t__RetCode__defined */


#ifndef __NDR_convert__int_rep__Reply__host_kernel_version_t__kernel_version__defined
#if	defined(__NDR_convert__int_rep__mach_host__kernel_version_t__defined)
#define	__NDR_convert__int_rep__Reply__host_kernel_version_t__kernel_version__defined
#define	__NDR_convert__int_rep__Reply__host_kernel_version_t__kernel_version(a, f, c) \
	__NDR_convert__int_rep__mach_host__kernel_version_t((kernel_version_t *)(a), f, c)
#elif	defined(__NDR_convert__int_rep__kernel_version_t__defined)
#define	__NDR_convert__int_rep__Reply__host_kernel_version_t__kernel_version__defined
#define	__NDR_convert__int_rep__Reply__host_kernel_version_t__kernel_version(a, f, c) \
	__NDR_convert__int_rep__kernel_version_t((kernel_version_t *)(a), f, c)
#endif /* defined(__NDR_convert__*__defined) */
#endif /* __NDR_convert__int_rep__Reply__host_kernel_version_t__kernel_version__defined */


#ifndef __NDR_convert__int_rep__Reply__host_kernel_version_t__kernel_versionCnt__defined
#if	defined(__NDR_convert__int_rep__mach_host__mach_msg_type_number_t__defined)
#define	__NDR_convert__int_rep__Reply__host_kernel_version_t__kernel_versionCnt__defined
#define	__NDR_convert__int_rep__Reply__host_kernel_version_t__kernel_versionCnt(a, f) \
	__NDR_convert__int_rep__mach_host__mach_msg_type_number_t((mach_msg_type_number_t *)(a), f)
#elif	defined(__NDR_convert__int_rep__mach_msg_type_number_t__defined)
#define	__NDR_convert__int_rep__Reply__host_kernel_version_t__kernel_versionCnt__defined
#define	__NDR_convert__int_rep__Reply__host_kernel_version_t__kernel_versionCnt(a, f) \
	__NDR_convert__int_rep__mach_msg_type_number_t((mach_msg_type_number_t *)(a), f)
#endif /* defined(__NDR_convert__*__defined) */
#endif /* __NDR_convert__int_rep__Reply__host_kernel_version_t__kernel_versionCnt__defined */



#ifndef __NDR_convert__char_rep__Reply__host_kernel_version_t__kernel_version__defined
#if	defined(__NDR_convert__char_rep__mach_host__kernel_version_t__defined)
#define	__NDR_convert__char_rep__Reply__host_kernel_version_t__kernel_version__defined
#define	__NDR_convert__char_rep__Reply__host_kernel_version_t__kernel_version(a, f, c) \
	__NDR_convert__char_rep__mach_host__kernel_version_t((kernel_version_t *)(a), f, c)
#elif	defined(__NDR_convert__char_rep__kernel_version_t__defined)
#define	__NDR_convert__char_rep__Reply__host_kernel_version_t__kernel_version__defined
#define	__NDR_convert__char_rep__Reply__host_kernel_version_t__kernel_version(a, f, c) \
	__NDR_convert__char_rep__kernel_version_t((kernel_version_t *)(a), f, c)
#endif /* defined(__NDR_convert__*__defined) */
#endif /* __NDR_convert__char_rep__Reply__host_kernel_version_t__kernel_version__defined */




#ifndef __NDR_convert__float_rep__Reply__host_kernel_version_t__kernel_version__defined
#if	defined(__NDR_convert__float_rep__mach_host__kernel_version_t__defined)
#define	__NDR_convert__float_rep__Reply__host_kernel_version_t__kernel_version__defined
#define	__NDR_convert__float_rep__Reply__host_kernel_version_t__kernel_version(a, f, c) \
	__NDR_convert__float_rep__mach_host__kernel_version_t((kernel_version_t *)(a), f, c)
#elif	defined(__NDR_convert__float_rep__kernel_version_t__defined)
#define	__NDR_convert__float_rep__Reply__host_kernel_version_t__kernel_version__defined
#define	__NDR_convert__float_rep__Reply__host_kernel_version_t__kernel_version(a, f, c) \
	__NDR_convert__float_rep__kernel_version_t((kernel_version_t *)(a), f, c)
#endif /* defined(__NDR_convert__*__defined) */
#endif /* __NDR_convert__float_rep__Reply__host_kernel_version_t__kernel_version__defined */




mig_internal kern_return_t __MIG_check__Reply__host_kernel_version_t(__Reply__host_kernel_version_t *Out0P)
{

	typedef __Reply__host_kernel_version_t __Reply;
#if	__MigTypeCheck
	unsigned int msgh_size;
#endif	/* __MigTypeCheck */

	if (Out0P->Head.msgh_id != 301) {
	    if (Out0P->Head.msgh_id == MACH_NOTIFY_SEND_ONCE)
		{ return MIG_SERVER_DIED; }
	    else
		{ return MIG_REPLY_MISMATCH; }
	}

#if	__MigTypeCheck
	msgh_size = Out0P->Head.msgh_size;

	if ((Out0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) ||
	    ((msgh_size > (mach_msg_size_t)sizeof(__Reply) || msgh_size < (mach_msg_size_t)(sizeof(__Reply) - 512)) &&
	     (msgh_size != (mach_msg_size_t)sizeof(mig_reply_error_t) ||
	      Out0P->RetCode == KERN_SUCCESS)))
		{ return MIG_TYPE_ERROR ; }
#endif	/* __MigTypeCheck */

	if (Out0P->RetCode != KERN_SUCCESS) {
#ifdef	__NDR_convert__mig_reply_error_t__defined
		__NDR_convert__mig_reply_error_t((mig_reply_error_t *)Out0P);
#endif	/* __NDR_convert__mig_reply_error_t__defined */
		return ((mig_reply_error_t *)Out0P)->RetCode;
	}

#if defined(__NDR_convert__int_rep__Reply__host_kernel_version_t__kernel_versionCnt__defined)
	if (Out0P->NDR.int_rep != NDR_record.int_rep)
		__NDR_convert__int_rep__Reply__host_kernel_version_t__kernel_versionCnt(&Out0P->kernel_versionCnt, Out0P->NDR.int_rep);
#endif	/* __NDR_convert__int_rep__Reply__host_kernel_version_t__kernel_versionCnt__defined */
#if	__MigTypeCheck
	if (msgh_size != (mach_msg_size_t)(sizeof(__Reply) - 512) + (_WALIGN_(Out0P->kernel_versionCnt)))
		{ return MIG_TYPE_ERROR ; }
#endif	/* __MigTypeCheck */

#if	defined(__NDR_convert__int_rep__Reply__host_kernel_version_t__RetCode__defined) || \
	defined(__NDR_convert__int_rep__Reply__host_kernel_version_t__kernel_version__defined) || \
	defined(__NDR_convert__int_rep__Reply__host_kernel_version_t__kernel_versionCnt__defined)
	if (Out0P->NDR.int_rep != NDR_record.int_rep) {
#if defined(__NDR_convert__int_rep__Reply__host_kernel_version_t__RetCode__defined)
		__NDR_convert__int_rep__Reply__host_kernel_version_t__RetCode(&Out0P->RetCode, Out0P->NDR.int_rep);
#endif /* __NDR_convert__int_rep__Reply__host_kernel_version_t__RetCode__defined */
#if defined(__NDR_convert__int_rep__Reply__host_kernel_version_t__kernel_version__defined)
		__NDR_convert__int_rep__Reply__host_kernel_version_t__kernel_version(&Out0P->kernel_version, Out0P->NDR.int_rep, Out0P->kernel_versionCnt);
#endif /* __NDR_convert__int_rep__Reply__host_kernel_version_t__kernel_version__defined */
	}
#endif	/* defined(__NDR_convert__int_rep...) */

#if	0 || \
	defined(__NDR_convert__char_rep__Reply__host_kernel_version_t__kernel_version__defined) || \
	0
	if (Out0P->NDR.char_rep != NDR_record.char_rep) {
#if defined(__NDR_convert__char_rep__Reply__host_kernel_version_t__kernel_version__defined)
		__NDR_convert__char_rep__Reply__host_kernel_version_t__kernel_version(&Out0P->kernel_version, Out0P->NDR.char_rep, Out0P->kernel_versionCnt);
#endif /* __NDR_convert__char_rep__Reply__host_kernel_version_t__kernel_version__defined */
	}
#endif	/* defined(__NDR_convert__char_rep...) */

#if	0 || \
	defined(__NDR_convert__float_rep__Reply__host_kernel_version_t__kernel_version__defined) || \
	0
	if (Out0P->NDR.float_rep != NDR_record.float_rep) {
#if defined(__NDR_convert__float_rep__Reply__host_kernel_version_t__kernel_version__defined)
		__NDR_convert__float_rep__Reply__host_kernel_version_t__kernel_version(&Out0P->kernel_version, Out0P->NDR.float_rep, Out0P->kernel_versionCnt);
#endif /* __NDR_convert__float_rep__Reply__host_kernel_version_t__kernel_version__defined */
	}
#endif	/* defined(__NDR_convert__float_rep...) */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Reply__host_kernel_version_t__defined) */
#endif /* __MIG_check__Reply__mach_host_subsystem__ */
#endif /* ( __MigTypeCheck || __NDR_convert__ ) */


/* Routine host_kernel_version */
mig_external kern_return_t host_kernel_version
(
	host_t host,
	kernel_version_t kernel_version
)
{

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
	} Request;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		NDR_record_t NDR;
		kern_return_t RetCode;
		mach_msg_type_number_t kernel_versionOffset; /* MiG doesn't use it */
		mach_msg_type_number_t kernel_versionCnt;
		char kernel_version[512];
		mach_msg_trailer_t trailer;
	} Reply;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		NDR_record_t NDR;
		kern_return_t RetCode;
		mach_msg_type_number_t kernel_versionOffset; /* MiG doesn't use it */
		mach_msg_type_number_t kernel_versionCnt;
		char kernel_version[512];
	} __Reply;
#ifdef  __MigPackStructs
#pragma pack()
#endif
	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	union {
		Request In;
		Reply Out;
	} Mess;

	Request *InP = &Mess.In;
	Reply *Out0P = &Mess.Out;

	mach_msg_return_t msg_result;

#ifdef	__MIG_check__Reply__host_kernel_version_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Reply__host_kernel_version_t__defined */

	__DeclareSendRpc(201, "host_kernel_version")

	InP->Head.msgh_bits =
		MACH_MSGH_BITS(19, MACH_MSG_TYPE_MAKE_SEND_ONCE);
	/* msgh_size passed as argument */
	InP->Head.msgh_request_port = host;
	InP->Head.msgh_reply_port = mig_get_reply_port();
	InP->Head.msgh_id = 201;

	__BeforeSendRpc(201, "host_kernel_version")
	msg_result = mach_msg(&InP->Head, MACH_SEND_MSG|MACH_RCV_MSG|MACH_MSG_OPTION_NONE, (mach_msg_size_t)sizeof(Request), (mach_msg_size_t)sizeof(Reply), InP->Head.msgh_reply_port, MACH_MSG_TIMEOUT_NONE, MACH_PORT_NULL);
	__AfterSendRpc(201, "host_kernel_version")
	if (msg_result != MACH_MSG_SUCCESS) {
		__MachMsgErrorWithoutTimeout(msg_result);
		{ return msg_result; }
	}


#if	defined(__MIG_check__Reply__host_kernel_version_t__defined)
	check_result = __MIG_check__Reply__host_kernel_version_t((__Reply__host_kernel_version_t *)Out0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ return check_result; }
#endif	/* defined(__MIG_check__Reply__host_kernel_version_t__defined) */

	(void) mig_strncpy(kernel_version, Out0P->kernel_version, 512);

	return KERN_SUCCESS;
}

#if ( __MigTypeCheck || __NDR_convert__ )
#if __MIG_check__Reply__mach_host_subsystem__
#if !defined(__MIG_check__Reply___host_page_size_t__defined)
#define __MIG_check__Reply___host_page_size_t__defined
#ifndef __NDR_convert__int_rep__Reply___host_page_size_t__RetCode__defined
#if	defined(__NDR_convert__int_rep__mach_host__kern_return_t__defined)
#define	__NDR_convert__int_rep__Reply___host_page_size_t__RetCode__defined
#define	__NDR_convert__int_rep__Reply___host_page_size_t__RetCode(a, f) \
	__NDR_convert__int_rep__mach_host__kern_return_t((kern_return_t *)(a), f)
#elif	defined(__NDR_convert__int_rep__kern_return_t__defined)
#define	__NDR_convert__int_rep__Reply___host_page_size_t__RetCode__defined
#define	__NDR_convert__int_rep__Reply___host_page_size_t__RetCode(a, f) \
	__NDR_convert__int_rep__kern_return_t((kern_return_t *)(a), f)
#endif /* defined(__NDR_convert__*__defined) */
#endif /* __NDR_convert__int_rep__Reply___host_page_size_t__RetCode__defined */


#ifndef __NDR_convert__int_rep__Reply___host_page_size_t__out_page_size__defined
#if	defined(__NDR_convert__int_rep__mach_host__vm_size_t__defined)
#define	__NDR_convert__int_rep__Reply___host_page_size_t__out_page_size__defined
#define	__NDR_convert__int_rep__Reply___host_page_size_t__out_page_size(a, f) \
	__NDR_convert__int_rep__mach_host__vm_size_t((vm_size_t *)(a), f)
#elif	defined(__NDR_convert__int_rep__vm_size_t__defined)
#define	__NDR_convert__int_rep__Reply___host_page_size_t__out_page_size__defined
#define	__NDR_convert__int_rep__Reply___host_page_size_t__out_page_size(a, f) \
	__NDR_convert__int_rep__vm_size_t((vm_size_t *)(a), f)
#elif	defined(__NDR_convert__int_rep__mach_host__natural_t__defined)
#define	__NDR_convert__int_rep__Reply___host_page_size_t__out_page_size__defined
#define	__NDR_convert__int_rep__Reply___host_page_size_t__out_page_size(a, f) \
	__NDR_convert__int_rep__mach_host__natural_t((natural_t *)(a), f)
#elif	defined(__NDR_convert__int_rep__natural_t__defined)
#define	__NDR_convert__int_rep__Reply___host_page_size_t__out_page_size__defined
#define	__NDR_convert__int_rep__Reply___host_page_size_t__out_page_size(a, f) \
	__NDR_convert__int_rep__natural_t((natural_t *)(a), f)
#elif	defined(__NDR_convert__int_rep__mach_host__uint32_t__defined)
#define	__NDR_convert__int_rep__Reply___host_page_size_t__out_page_size__defined
#define	__NDR_convert__int_rep__Reply___host_page_size_t__out_page_size(a, f) \
	__NDR_convert__int_rep__mach_host__uint32_t((uint32_t *)(a), f)
#elif	defined(__NDR_convert__int_rep__uint32_t__defined)
#define	__NDR_convert__int_rep__Reply___host_page_size_t__out_page_size__defined
#define	__NDR_convert__int_rep__Reply___host_page_size_t__out_page_size(a, f) \
	__NDR_convert__int_rep__uint32_t((uint32_t *)(a), f)
#endif /* defined(__NDR_convert__*__defined) */
#endif /* __NDR_convert__int_rep__Reply___host_page_size_t__out_page_size__defined */



#ifndef __NDR_convert__char_rep__Reply___host_page_size_t__out_page_size__defined
#if	defined(__NDR_convert__char_rep__mach_host__vm_size_t__defined)
#define	__NDR_convert__char_rep__Reply___host_page_size_t__out_page_size__defined
#define	__NDR_convert__char_rep__Reply___host_page_size_t__out_page_size(a, f) \
	__NDR_convert__char_rep__mach_host__vm_size_t((vm_size_t *)(a), f)
#elif	defined(__NDR_convert__char_rep__vm_size_t__defined)
#define	__NDR_convert__char_rep__Reply___host_page_size_t__out_page_size__defined
#define	__NDR_convert__char_rep__Reply___host_page_size_t__out_page_size(a, f) \
	__NDR_convert__char_rep__vm_size_t((vm_size_t *)(a), f)
#elif	defined(__NDR_convert__char_rep__mach_host__natural_t__defined)
#define	__NDR_convert__char_rep__Reply___host_page_size_t__out_page_size__defined
#define	__NDR_convert__char_rep__Reply___host_page_size_t__out_page_size(a, f) \
	__NDR_convert__char_rep__mach_host__natural_t((natural_t *)(a), f)
#elif	defined(__NDR_convert__char_rep__natural_t__defined)
#define	__NDR_convert__char_rep__Reply___host_page_size_t__out_page_size__defined
#define	__NDR_convert__char_rep__Reply___host_page_size_t__out_page_size(a, f) \
	__NDR_convert__char_rep__natural_t((natural_t *)(a), f)
#elif	defined(__NDR_convert__char_rep__mach_host__uint32_t__defined)
#define	__NDR_convert__char_rep__Reply___host_page_size_t__out_page_size__defined
#define	__NDR_convert__char_rep__Reply___host_page_size_t__out_page_size(a, f) \
	__NDR_convert__char_rep__mach_host__uint32_t((uint32_t *)(a), f)
#elif	defined(__NDR_convert__char_rep__uint32_t__defined)
#define	__NDR_convert__char_rep__Reply___host_page_size_t__out_page_size__defined
#define	__NDR_convert__char_rep__Reply___host_page_size_t__out_page_size(a, f) \
	__NDR_convert__char_rep__uint32_t((uint32_t *)(a), f)
#endif /* defined(__NDR_convert__*__defined) */
#endif /* __NDR_convert__char_rep__Reply___host_page_size_t__out_page_size__defined */



#ifndef __NDR_convert__float_rep__Reply___host_page_size_t__out_page_size__defined
#if	defined(__NDR_convert__float_rep__mach_host__vm_size_t__defined)
#define	__NDR_convert__float_rep__Reply___host_page_size_t__out_page_size__defined
#define	__NDR_convert__float_rep__Reply___host_page_size_t__out_page_size(a, f) \
	__NDR_convert__float_rep__mach_host__vm_size_t((vm_size_t *)(a), f)
#elif	defined(__NDR_convert__float_rep__vm_size_t__defined)
#define	__NDR_convert__float_rep__Reply___host_page_size_t__out_page_size__defined
#define	__NDR_convert__float_rep__Reply___host_page_size_t__out_page_size(a, f) \
	__NDR_convert__float_rep__vm_size_t((vm_size_t *)(a), f)
#elif	defined(__NDR_convert__float_rep__mach_host__natural_t__defined)
#define	__NDR_convert__float_rep__Reply___host_page_size_t__out_page_size__defined
#define	__NDR_convert__float_rep__Reply___host_page_size_t__out_page_size(a, f) \
	__NDR_convert__float_rep__mach_host__natural_t((natural_t *)(a), f)
#elif	defined(__NDR_convert__float_rep__natural_t__defined)
#define	__NDR_convert__float_rep__Reply___host_page_size_t__out_page_size__defined
#define	__NDR_convert__float_rep__Reply___host_page_size_t__out_page_size(a, f) \
	__NDR_convert__float_rep__natural_t((natural_t *)(a), f)
#elif	defined(__NDR_convert__float_rep__mach_host__uint32_t__defined)
#define	__NDR_convert__float_rep__Reply___host_page_size_t__out_page_size__defined
#define	__NDR_convert__float_rep__Reply___host_page_size_t__out_page_size(a, f) \
	__NDR_convert__float_rep__mach_host__uint32_t((uint32_t *)(a), f)
#elif	defined(__NDR_convert__float_rep__uint32_t__defined)
#define	__NDR_convert__float_rep__Reply___host_page_size_t__out_page_size__defined
#define	__NDR_convert__float_rep__Reply___host_page_size_t__out_page_size(a, f) \
	__NDR_convert__float_rep__uint32_t((uint32_t *)(a), f)
#endif /* defined(__NDR_convert__*__defined) */
#endif /* __NDR_convert__float_rep__Reply___host_page_size_t__out_page_size__defined */



mig_internal kern_return_t __MIG_check__Reply___host_page_size_t(__Reply___host_page_size_t *Out0P)
{

	typedef __Reply___host_page_size_t __Reply;
#if	__MigTypeCheck
	unsigned int msgh_size;
#endif	/* __MigTypeCheck */
	if (Out0P->Head.msgh_id != 302) {
	    if (Out0P->Head.msgh_id == MACH_NOTIFY_SEND_ONCE)
		{ return MIG_SERVER_DIED; }
	    else
		{ return MIG_REPLY_MISMATCH; }
	}

#if	__MigTypeCheck
	msgh_size = Out0P->Head.msgh_size;

	if ((Out0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) ||
	    ((msgh_size != (mach_msg_size_t)sizeof(__Reply)) &&
	     (msgh_size != (mach_msg_size_t)sizeof(mig_reply_error_t) ||
	      Out0P->RetCode == KERN_SUCCESS)))
		{ return MIG_TYPE_ERROR ; }
#endif	/* __MigTypeCheck */

	if (Out0P->RetCode != KERN_SUCCESS) {
#ifdef	__NDR_convert__mig_reply_error_t__defined
		__NDR_convert__mig_reply_error_t((mig_reply_error_t *)Out0P);
#endif	/* __NDR_convert__mig_reply_error_t__defined */
		return ((mig_reply_error_t *)Out0P)->RetCode;
	}

#if	defined(__NDR_convert__int_rep__Reply___host_page_size_t__RetCode__defined) || \
	defined(__NDR_convert__int_rep__Reply___host_page_size_t__out_page_size__defined)
	if (Out0P->NDR.int_rep != NDR_record.int_rep) {
#if defined(__NDR_convert__int_rep__Reply___host_page_size_t__RetCode__defined)
		__NDR_convert__int_rep__Reply___host_page_size_t__RetCode(&Out0P->RetCode, Out0P->NDR.int_rep);
#endif /* __NDR_convert__int_rep__Reply___host_page_size_t__RetCode__defined */
#if defined(__NDR_convert__int_rep__Reply___host_page_size_t__out_page_size__defined)
		__NDR_convert__int_rep__Reply___host_page_size_t__out_page_size(&Out0P->out_page_size, Out0P->NDR.int_rep);
#endif /* __NDR_convert__int_rep__Reply___host_page_size_t__out_page_size__defined */
	}
#endif	/* defined(__NDR_convert__int_rep...) */

#if	0 || \
	defined(__NDR_convert__char_rep__Reply___host_page_size_t__out_page_size__defined)
	if (Out0P->NDR.char_rep != NDR_record.char_rep) {
#if defined(__NDR_convert__char_rep__Reply___host_page_size_t__out_page_size__defined)
		__NDR_convert__char_rep__Reply___host_page_size_t__out_page_size(&Out0P->out_page_size, Out0P->NDR.char_rep);
#endif /* __NDR_convert__char_rep__Reply___host_page_size_t__out_page_size__defined */
	}
#endif	/* defined(__NDR_convert__char_rep...) */

#if	0 || \
	defined(__NDR_convert__float_rep__Reply___host_page_size_t__out_page_size__defined)
	if (Out0P->NDR.float_rep != NDR_record.float_rep) {
#if defined(__NDR_convert__float_rep__Reply___host_page_size_t__out_page_size__defined)
		__NDR_convert__float_rep__Reply___host_page_size_t__out_page_size(&Out0P->out_page_size, Out0P->NDR.float_rep);
#endif /* __NDR_convert__float_rep__Reply___host_page_size_t__out_page_size__defined */
	}
#endif	/* defined(__NDR_convert__float_rep...) */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Reply___host_page_size_t__defined) */
#endif /* __MIG_check__Reply__mach_host_subsystem__ */
#endif /* ( __MigTypeCheck || __NDR_convert__ ) */


/* Routine _host_page_size */
mig_external kern_return_t _host_page_size
(
	host_t host,
	vm_size_t *out_page_size
)
{

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
	} Request;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		NDR_record_t NDR;
		kern_return_t RetCode;
		vm_size_t out_page_size;
		mach_msg_trailer_t trailer;
	} Reply;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		NDR_record_t NDR;
		kern_return_t RetCode;
		vm_size_t out_page_size;
	} __Reply;
#ifdef  __MigPackStructs
#pragma pack()
#endif
	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	union {
		Request In;
		Reply Out;
	} Mess;

	Request *InP = &Mess.In;
	Reply *Out0P = &Mess.Out;

	mach_msg_return_t msg_result;

#ifdef	__MIG_check__Reply___host_page_size_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Reply___host_page_size_t__defined */

	__DeclareSendRpc(202, "_host_page_size")

	InP->Head.msgh_bits =
		MACH_MSGH_BITS(19, MACH_MSG_TYPE_MAKE_SEND_ONCE);
	/* msgh_size passed as argument */
	InP->Head.msgh_request_port = host;
	InP->Head.msgh_reply_port = mig_get_reply_port();
	InP->Head.msgh_id = 202;

	__BeforeSendRpc(202, "_host_page_size")
	msg_result = mach_msg(&InP->Head, MACH_SEND_MSG|MACH_RCV_MSG|MACH_MSG_OPTION_NONE, (mach_msg_size_t)sizeof(Request), (mach_msg_size_t)sizeof(Reply), InP->Head.msgh_reply_port, MACH_MSG_TIMEOUT_NONE, MACH_PORT_NULL);
	__AfterSendRpc(202, "_host_page_size")
	if (msg_result != MACH_MSG_SUCCESS) {
		__MachMsgErrorWithoutTimeout(msg_result);
		{ return msg_result; }
	}


#if	defined(__MIG_check__Reply___host_page_size_t__defined)
	check_result = __MIG_check__Reply___host_page_size_t((__Reply___host_page_size_t *)Out0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ return check_result; }
#endif	/* defined(__MIG_check__Reply___host_page_size_t__defined) */

	*out_page_size = Out0P->out_page_size;

	return KERN_SUCCESS;
}

#if ( __MigTypeCheck || __NDR_convert__ )
#if __MIG_check__Reply__mach_host_subsystem__
#if !defined(__MIG_check__Reply__mach_memory_object_memory_entry_t__defined)
#define __MIG_check__Reply__mach_memory_object_memory_entry_t__defined

mig_internal kern_return_t __MIG_check__Reply__mach_memory_object_memory_entry_t(__Reply__mach_memory_object_memory_entry_t *Out0P)
{

	typedef __Reply__mach_memory_object_memory_entry_t __Reply;
	boolean_t msgh_simple;
#if	__MigTypeCheck
	unsigned int msgh_size;
#endif	/* __MigTypeCheck */
	if (Out0P->Head.msgh_id != 303) {
	    if (Out0P->Head.msgh_id == MACH_NOTIFY_SEND_ONCE)
		{ return MIG_SERVER_DIED; }
	    else
		{ return MIG_REPLY_MISMATCH; }
	}

	msgh_simple = !(Out0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX);
#if	__MigTypeCheck
	msgh_size = Out0P->Head.msgh_size;

	if ((msgh_simple || Out0P->msgh_body.msgh_descriptor_count != 1 ||
	    msgh_size != (mach_msg_size_t)sizeof(__Reply)) &&
	    (!msgh_simple || msgh_size != (mach_msg_size_t)sizeof(mig_reply_error_t) ||
	    ((mig_reply_error_t *)Out0P)->RetCode == KERN_SUCCESS))
		{ return MIG_TYPE_ERROR ; }
#endif	/* __MigTypeCheck */

	if (msgh_simple) {
#ifdef	__NDR_convert__mig_reply_error_t__defined
		__NDR_convert__mig_reply_error_t((mig_reply_error_t *)Out0P);
#endif	/* __NDR_convert__mig_reply_error_t__defined */
		return ((mig_reply_error_t *)Out0P)->RetCode;
	}

#if	__MigTypeCheck
	if (Out0P->entry_handle.type != MACH_MSG_PORT_DESCRIPTOR ||
	    Out0P->entry_handle.disposition != 17) {
		return MIG_TYPE_ERROR;
	}
#endif	/* __MigTypeCheck */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Reply__mach_memory_object_memory_entry_t__defined) */
#endif /* __MIG_check__Reply__mach_host_subsystem__ */
#endif /* ( __MigTypeCheck || __NDR_convert__ ) */


/* Routine mach_memory_object_memory_entry */
mig_external kern_return_t mach_memory_object_memory_entry
(
	host_t host,
	boolean_t internal,
	vm_size_t size,
	vm_prot_t permission,
	memory_object_t pager,
	mach_port_t *entry_handle
)
{

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		mach_msg_port_descriptor_t pager;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		boolean_t internal;
		vm_size_t size;
		vm_prot_t permission;
	} Request;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		mach_msg_port_descriptor_t entry_handle;
		/* end of the kernel processed data */
		mach_msg_trailer_t trailer;
	} Reply;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		mach_msg_port_descriptor_t entry_handle;
		/* end of the kernel processed data */
	} __Reply;
#ifdef  __MigPackStructs
#pragma pack()
#endif
	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	union {
		Request In;
		Reply Out;
	} Mess;

	Request *InP = &Mess.In;
	Reply *Out0P = &Mess.Out;

	mach_msg_return_t msg_result;

#ifdef	__MIG_check__Reply__mach_memory_object_memory_entry_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Reply__mach_memory_object_memory_entry_t__defined */

	__DeclareSendRpc(203, "mach_memory_object_memory_entry")

#if	UseStaticTemplates
	const static mach_msg_port_descriptor_t pagerTemplate = {
		/* name = */		MACH_PORT_NULL,
		/* pad1 = */		0,
		/* pad2 = */		0,
		/* disp = */		19,
		/* type = */		MACH_MSG_PORT_DESCRIPTOR,
	};
#endif	/* UseStaticTemplates */

	InP->msgh_body.msgh_descriptor_count = 1;
#if	UseStaticTemplates
	InP->pager = pagerTemplate;
	InP->pager.name = pager;
#else	/* UseStaticTemplates */
	InP->pager.name = pager;
	InP->pager.disposition = 19;
	InP->pager.type = MACH_MSG_PORT_DESCRIPTOR;
#endif	/* UseStaticTemplates */

	InP->NDR = NDR_record;

	InP->internal = internal;

	InP->size = size;

	InP->permission = permission;

	InP->Head.msgh_bits = MACH_MSGH_BITS_COMPLEX|
		MACH_MSGH_BITS(19, MACH_MSG_TYPE_MAKE_SEND_ONCE);
	/* msgh_size passed as argument */
	InP->Head.msgh_request_port = host;
	InP->Head.msgh_reply_port = mig_get_reply_port();
	InP->Head.msgh_id = 203;

	__BeforeSendRpc(203, "mach_memory_object_memory_entry")
	msg_result = mach_msg(&InP->Head, MACH_SEND_MSG|MACH_RCV_MSG|MACH_MSG_OPTION_NONE, (mach_msg_size_t)sizeof(Request), (mach_msg_size_t)sizeof(Reply), InP->Head.msgh_reply_port, MACH_MSG_TIMEOUT_NONE, MACH_PORT_NULL);
	__AfterSendRpc(203, "mach_memory_object_memory_entry")
	if (msg_result != MACH_MSG_SUCCESS) {
		__MachMsgErrorWithoutTimeout(msg_result);
		{ return msg_result; }
	}


#if	defined(__MIG_check__Reply__mach_memory_object_memory_entry_t__defined)
	check_result = __MIG_check__Reply__mach_memory_object_memory_entry_t((__Reply__mach_memory_object_memory_entry_t *)Out0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ return check_result; }
#endif	/* defined(__MIG_check__Reply__mach_memory_object_memory_entry_t__defined) */

	*entry_handle = Out0P->entry_handle.name;
	return KERN_SUCCESS;
}

#if ( __MigTypeCheck || __NDR_convert__ )
#if __MIG_check__Reply__mach_host_subsystem__
#if !defined(__MIG_check__Reply__host_processor_info_t__defined)
#define __MIG_check__Reply__host_processor_info_t__defined
#ifndef __NDR_convert__int_rep__Reply__host_processor_info_t__out_processor_count__defined
#if	defined(__NDR_convert__int_rep__mach_host__natural_t__defined)
#define	__NDR_convert__int_rep__Reply__host_processor_info_t__out_processor_count__defined
#define	__NDR_convert__int_rep__Reply__host_processor_info_t__out_processor_count(a, f) \
	__NDR_convert__int_rep__mach_host__natural_t((natural_t *)(a), f)
#elif	defined(__NDR_convert__int_rep__natural_t__defined)
#define	__NDR_convert__int_rep__Reply__host_processor_info_t__out_processor_count__defined
#define	__NDR_convert__int_rep__Reply__host_processor_info_t__out_processor_count(a, f) \
	__NDR_convert__int_rep__natural_t((natural_t *)(a), f)
#elif	defined(__NDR_convert__int_rep__mach_host__uint32_t__defined)
#define	__NDR_convert__int_rep__Reply__host_processor_info_t__out_processor_count__defined
#define	__NDR_convert__int_rep__Reply__host_processor_info_t__out_processor_count(a, f) \
	__NDR_convert__int_rep__mach_host__uint32_t((uint32_t *)(a), f)
#elif	defined(__NDR_convert__int_rep__uint32_t__defined)
#define	__NDR_convert__int_rep__Reply__host_processor_info_t__out_processor_count__defined
#define	__NDR_convert__int_rep__Reply__host_processor_info_t__out_processor_count(a, f) \
	__NDR_convert__int_rep__uint32_t((uint32_t *)(a), f)
#endif /* defined(__NDR_convert__*__defined) */
#endif /* __NDR_convert__int_rep__Reply__host_processor_info_t__out_processor_count__defined */


#ifndef __NDR_convert__int_rep__Reply__host_processor_info_t__out_processor_info__defined
#if	defined(__NDR_convert__int_rep__mach_host__processor_info_array_t__defined)
#define	__NDR_convert__int_rep__Reply__host_processor_info_t__out_processor_info__defined
#define	__NDR_convert__int_rep__Reply__host_processor_info_t__out_processor_info(a, f, c) \
	__NDR_convert__int_rep__mach_host__processor_info_array_t((processor_info_array_t *)(a), f, c)
#elif	defined(__NDR_convert__int_rep__processor_info_array_t__defined)
#define	__NDR_convert__int_rep__Reply__host_processor_info_t__out_processor_info__defined
#define	__NDR_convert__int_rep__Reply__host_processor_info_t__out_processor_info(a, f, c) \
	__NDR_convert__int_rep__processor_info_array_t((processor_info_array_t *)(a), f, c)
#elif	defined(__NDR_convert__int_rep__mach_host__integer_t__defined)
#define	__NDR_convert__int_rep__Reply__host_processor_info_t__out_processor_info__defined
#define	__NDR_convert__int_rep__Reply__host_processor_info_t__out_processor_info(a, f, c) \
	__NDR_convert__ARRAY((integer_t *)(a), f, c, __NDR_convert__int_rep__mach_host__integer_t)
#elif	defined(__NDR_convert__int_rep__integer_t__defined)
#define	__NDR_convert__int_rep__Reply__host_processor_info_t__out_processor_info__defined
#define	__NDR_convert__int_rep__Reply__host_processor_info_t__out_processor_info(a, f, c) \
	__NDR_convert__ARRAY((integer_t *)(a), f, c, __NDR_convert__int_rep__integer_t)
#elif	defined(__NDR_convert__int_rep__mach_host__int32_t__defined)
#define	__NDR_convert__int_rep__Reply__host_processor_info_t__out_processor_info__defined
#define	__NDR_convert__int_rep__Reply__host_processor_info_t__out_processor_info(a, f, c) \
	__NDR_convert__ARRAY((int32_t *)(a), f, c, __NDR_convert__int_rep__mach_host__int32_t)
#elif	defined(__NDR_convert__int_rep__int32_t__defined)
#define	__NDR_convert__int_rep__Reply__host_processor_info_t__out_processor_info__defined
#define	__NDR_convert__int_rep__Reply__host_processor_info_t__out_processor_info(a, f, c) \
	__NDR_convert__ARRAY((int32_t *)(a), f, c, __NDR_convert__int_rep__int32_t)
#endif /* defined(__NDR_convert__*__defined) */
#endif /* __NDR_convert__int_rep__Reply__host_processor_info_t__out_processor_info__defined */


#ifndef __NDR_convert__int_rep__Reply__host_processor_info_t__out_processor_infoCnt__defined
#if	defined(__NDR_convert__int_rep__mach_host__mach_msg_type_number_t__defined)
#define	__NDR_convert__int_rep__Reply__host_processor_info_t__out_processor_infoCnt__defined
#define	__NDR_convert__int_rep__Reply__host_processor_info_t__out_processor_infoCnt(a, f) \
	__NDR_convert__int_rep__mach_host__mach_msg_type_number_t((mach_msg_type_number_t *)(a), f)
#elif	defined(__NDR_convert__int_rep__mach_msg_type_number_t__defined)
#define	__NDR_convert__int_rep__Reply__host_processor_info_t__out_processor_infoCnt__defined
#define	__NDR_convert__int_rep__Reply__host_processor_info_t__out_processor_infoCnt(a, f) \
	__NDR_convert__int_rep__mach_msg_type_number_t((mach_msg_type_number_t *)(a), f)
#endif /* defined(__NDR_convert__*__defined) */
#endif /* __NDR_convert__int_rep__Reply__host_processor_info_t__out_processor_infoCnt__defined */


#ifndef __NDR_convert__char_rep__Reply__host_processor_info_t__out_processor_count__defined
#if	defined(__NDR_convert__char_rep__mach_host__natural_t__defined)
#define	__NDR_convert__char_rep__Reply__host_processor_info_t__out_processor_count__defined
#define	__NDR_convert__char_rep__Reply__host_processor_info_t__out_processor_count(a, f) \
	__NDR_convert__char_rep__mach_host__natural_t((natural_t *)(a), f)
#elif	defined(__NDR_convert__char_rep__natural_t__defined)
#define	__NDR_convert__char_rep__Reply__host_processor_info_t__out_processor_count__defined
#define	__NDR_convert__char_rep__Reply__host_processor_info_t__out_processor_count(a, f) \
	__NDR_convert__char_rep__natural_t((natural_t *)(a), f)
#elif	defined(__NDR_convert__char_rep__mach_host__uint32_t__defined)
#define	__NDR_convert__char_rep__Reply__host_processor_info_t__out_processor_count__defined
#define	__NDR_convert__char_rep__Reply__host_processor_info_t__out_processor_count(a, f) \
	__NDR_convert__char_rep__mach_host__uint32_t((uint32_t *)(a), f)
#elif	defined(__NDR_convert__char_rep__uint32_t__defined)
#define	__NDR_convert__char_rep__Reply__host_processor_info_t__out_processor_count__defined
#define	__NDR_convert__char_rep__Reply__host_processor_info_t__out_processor_count(a, f) \
	__NDR_convert__char_rep__uint32_t((uint32_t *)(a), f)
#endif /* defined(__NDR_convert__*__defined) */
#endif /* __NDR_convert__char_rep__Reply__host_processor_info_t__out_processor_count__defined */


#ifndef __NDR_convert__char_rep__Reply__host_processor_info_t__out_processor_info__defined
#if	defined(__NDR_convert__char_rep__mach_host__processor_info_array_t__defined)
#define	__NDR_convert__char_rep__Reply__host_processor_info_t__out_processor_info__defined
#define	__NDR_convert__char_rep__Reply__host_processor_info_t__out_processor_info(a, f, c) \
	__NDR_convert__char_rep__mach_host__processor_info_array_t((processor_info_array_t *)(a), f, c)
#elif	defined(__NDR_convert__char_rep__processor_info_array_t__defined)
#define	__NDR_convert__char_rep__Reply__host_processor_info_t__out_processor_info__defined
#define	__NDR_convert__char_rep__Reply__host_processor_info_t__out_processor_info(a, f, c) \
	__NDR_convert__char_rep__processor_info_array_t((processor_info_array_t *)(a), f, c)
#elif	defined(__NDR_convert__char_rep__mach_host__integer_t__defined)
#define	__NDR_convert__char_rep__Reply__host_processor_info_t__out_processor_info__defined
#define	__NDR_convert__char_rep__Reply__host_processor_info_t__out_processor_info(a, f, c) \
	__NDR_convert__ARRAY((integer_t *)(a), f, c, __NDR_convert__char_rep__mach_host__integer_t)
#elif	defined(__NDR_convert__char_rep__integer_t__defined)
#define	__NDR_convert__char_rep__Reply__host_processor_info_t__out_processor_info__defined
#define	__NDR_convert__char_rep__Reply__host_processor_info_t__out_processor_info(a, f, c) \
	__NDR_convert__ARRAY((integer_t *)(a), f, c, __NDR_convert__char_rep__integer_t)
#elif	defined(__NDR_convert__char_rep__mach_host__int32_t__defined)
#define	__NDR_convert__char_rep__Reply__host_processor_info_t__out_processor_info__defined
#define	__NDR_convert__char_rep__Reply__host_processor_info_t__out_processor_info(a, f, c) \
	__NDR_convert__ARRAY((int32_t *)(a), f, c, __NDR_convert__char_rep__mach_host__int32_t)
#elif	defined(__NDR_convert__char_rep__int32_t__defined)
#define	__NDR_convert__char_rep__Reply__host_processor_info_t__out_processor_info__defined
#define	__NDR_convert__char_rep__Reply__host_processor_info_t__out_processor_info(a, f, c) \
	__NDR_convert__ARRAY((int32_t *)(a), f, c, __NDR_convert__char_rep__int32_t)
#endif /* defined(__NDR_convert__*__defined) */
#endif /* __NDR_convert__char_rep__Reply__host_processor_info_t__out_processor_info__defined */



#ifndef __NDR_convert__float_rep__Reply__host_processor_info_t__out_processor_count__defined
#if	defined(__NDR_convert__float_rep__mach_host__natural_t__defined)
#define	__NDR_convert__float_rep__Reply__host_processor_info_t__out_processor_count__defined
#define	__NDR_convert__float_rep__Reply__host_processor_info_t__out_processor_count(a, f) \
	__NDR_convert__float_rep__mach_host__natural_t((natural_t *)(a), f)
#elif	defined(__NDR_convert__float_rep__natural_t__defined)
#define	__NDR_convert__float_rep__Reply__host_processor_info_t__out_processor_count__defined
#define	__NDR_convert__float_rep__Reply__host_processor_info_t__out_processor_count(a, f) \
	__NDR_convert__float_rep__natural_t((natural_t *)(a), f)
#elif	defined(__NDR_convert__float_rep__mach_host__uint32_t__defined)
#define	__NDR_convert__float_rep__Reply__host_processor_info_t__out_processor_count__defined
#define	__NDR_convert__float_rep__Reply__host_processor_info_t__out_processor_count(a, f) \
	__NDR_convert__float_rep__mach_host__uint32_t((uint32_t *)(a), f)
#elif	defined(__NDR_convert__float_rep__uint32_t__defined)
#define	__NDR_convert__float_rep__Reply__host_processor_info_t__out_processor_count__defined
#define	__NDR_convert__float_rep__Reply__host_processor_info_t__out_processor_count(a, f) \
	__NDR_convert__float_rep__uint32_t((uint32_t *)(a), f)
#endif /* defined(__NDR_convert__*__defined) */
#endif /* __NDR_convert__float_rep__Reply__host_processor_info_t__out_processor_count__defined */


#ifndef __NDR_convert__float_rep__Reply__host_processor_info_t__out_processor_info__defined
#if	defined(__NDR_convert__float_rep__mach_host__processor_info_array_t__defined)
#define	__NDR_convert__float_rep__Reply__host_processor_info_t__out_processor_info__defined
#define	__NDR_convert__float_rep__Reply__host_processor_info_t__out_processor_info(a, f, c) \
	__NDR_convert__float_rep__mach_host__processor_info_array_t((processor_info_array_t *)(a), f, c)
#elif	defined(__NDR_convert__float_rep__processor_info_array_t__defined)
#define	__NDR_convert__float_rep__Reply__host_processor_info_t__out_processor_info__defined
#define	__NDR_convert__float_rep__Reply__host_processor_info_t__out_processor_info(a, f, c) \
	__NDR_convert__float_rep__processor_info_array_t((processor_info_array_t *)(a), f, c)
#elif	defined(__NDR_convert__float_rep__mach_host__integer_t__defined)
#define	__NDR_convert__float_rep__Reply__host_processor_info_t__out_processor_info__defined
#define	__NDR_convert__float_rep__Reply__host_processor_info_t__out_processor_info(a, f, c) \
	__NDR_convert__ARRAY((integer_t *)(a), f, c, __NDR_convert__float_rep__mach_host__integer_t)
#elif	defined(__NDR_convert__float_rep__integer_t__defined)
#define	__NDR_convert__float_rep__Reply__host_processor_info_t__out_processor_info__defined
#define	__NDR_convert__float_rep__Reply__host_processor_info_t__out_processor_info(a, f, c) \
	__NDR_convert__ARRAY((integer_t *)(a), f, c, __NDR_convert__float_rep__integer_t)
#elif	defined(__NDR_convert__float_rep__mach_host__int32_t__defined)
#define	__NDR_convert__float_rep__Reply__host_processor_info_t__out_processor_info__defined
#define	__NDR_convert__float_rep__Reply__host_processor_info_t__out_processor_info(a, f, c) \
	__NDR_convert__ARRAY((int32_t *)(a), f, c, __NDR_convert__float_rep__mach_host__int32_t)
#elif	defined(__NDR_convert__float_rep__int32_t__defined)
#define	__NDR_convert__float_rep__Reply__host_processor_info_t__out_processor_info__defined
#define	__NDR_convert__float_rep__Reply__host_processor_info_t__out_processor_info(a, f, c) \
	__NDR_convert__ARRAY((int32_t *)(a), f, c, __NDR_convert__float_rep__int32_t)
#endif /* defined(__NDR_convert__*__defined) */
#endif /* __NDR_convert__float_rep__Reply__host_processor_info_t__out_processor_info__defined */




mig_internal kern_return_t __MIG_check__Reply__host_processor_info_t(__Reply__host_processor_info_t *Out0P)
{

	typedef __Reply__host_processor_info_t __Reply;
	boolean_t msgh_simple;
#if	__MigTypeCheck
	unsigned int msgh_size;
#endif	/* __MigTypeCheck */
	if (Out0P->Head.msgh_id != 304) {
	    if (Out0P->Head.msgh_id == MACH_NOTIFY_SEND_ONCE)
		{ return MIG_SERVER_DIED; }
	    else
		{ return MIG_REPLY_MISMATCH; }
	}

	msgh_simple = !(Out0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX);
#if	__MigTypeCheck
	msgh_size = Out0P->Head.msgh_size;

	if ((msgh_simple || Out0P->msgh_body.msgh_descriptor_count != 1 ||
	    msgh_size != (mach_msg_size_t)sizeof(__Reply)) &&
	    (!msgh_simple || msgh_size != (mach_msg_size_t)sizeof(mig_reply_error_t) ||
	    ((mig_reply_error_t *)Out0P)->RetCode == KERN_SUCCESS))
		{ return MIG_TYPE_ERROR ; }
#endif	/* __MigTypeCheck */

	if (msgh_simple) {
#ifdef	__NDR_convert__mig_reply_error_t__defined
		__NDR_convert__mig_reply_error_t((mig_reply_error_t *)Out0P);
#endif	/* __NDR_convert__mig_reply_error_t__defined */
		return ((mig_reply_error_t *)Out0P)->RetCode;
	}

#if	__MigTypeCheck
	if (Out0P->out_processor_info.type != MACH_MSG_OOL_DESCRIPTOR) {
		return MIG_TYPE_ERROR;
	}
#endif	/* __MigTypeCheck */

#if	defined(__NDR_convert__int_rep__Reply__host_processor_info_t__out_processor_count__defined) || \
	defined(__NDR_convert__int_rep__Reply__host_processor_info_t__out_processor_info__defined) || \
	defined(__NDR_convert__int_rep__Reply__host_processor_info_t__out_processor_infoCnt__defined)
	if (Out0P->NDR.int_rep != NDR_record.int_rep) {
#if defined(__NDR_convert__int_rep__Reply__host_processor_info_t__out_processor_count__defined)
		__NDR_convert__int_rep__Reply__host_processor_info_t__out_processor_count(&Out0P->out_processor_count, Out0P->NDR.int_rep);
#endif /* __NDR_convert__int_rep__Reply__host_processor_info_t__out_processor_count__defined */
#if defined(__NDR_convert__int_rep__Reply__host_processor_info_t__out_processor_infoCnt__defined)
		__NDR_convert__int_rep__Reply__host_processor_info_t__out_processor_infoCnt(&Out0P->out_processor_infoCnt, Out0P->NDR.int_rep);
#endif	/* __NDR_convert__int_rep__Reply__host_processor_info_t__out_processor_infoCnt__defined */
#if defined(__NDR_convert__int_rep__Reply__host_processor_info_t__out_processor_info__defined)
		__NDR_convert__int_rep__Reply__host_processor_info_t__out_processor_info((processor_info_array_t)(Out0P->out_processor_info.address), Out0P->NDR.int_rep, Out0P->out_processor_infoCnt);
#endif /* __NDR_convert__int_rep__Reply__host_processor_info_t__out_processor_info__defined */
	}
#endif	/* defined(__NDR_convert__int_rep...) */

#if	defined(__NDR_convert__char_rep__Reply__host_processor_info_t__out_processor_count__defined) || \
	defined(__NDR_convert__char_rep__Reply__host_processor_info_t__out_processor_info__defined) || \
	0
	if (Out0P->NDR.char_rep != NDR_record.char_rep) {
#if defined(__NDR_convert__char_rep__Reply__host_processor_info_t__out_processor_count__defined)
		__NDR_convert__char_rep__Reply__host_processor_info_t__out_processor_count(&Out0P->out_processor_count, Out0P->NDR.char_rep);
#endif /* __NDR_convert__char_rep__Reply__host_processor_info_t__out_processor_count__defined */
#if defined(__NDR_convert__char_rep__Reply__host_processor_info_t__out_processor_info__defined)
		__NDR_convert__char_rep__Reply__host_processor_info_t__out_processor_info((processor_info_array_t)(Out0P->out_processor_info.address), Out0P->NDR.char_rep, Out0P->out_processor_infoCnt);
#endif /* __NDR_convert__char_rep__Reply__host_processor_info_t__out_processor_info__defined */
	}
#endif	/* defined(__NDR_convert__char_rep...) */

#if	defined(__NDR_convert__float_rep__Reply__host_processor_info_t__out_processor_count__defined) || \
	defined(__NDR_convert__float_rep__Reply__host_processor_info_t__out_processor_info__defined) || \
	0
	if (Out0P->NDR.float_rep != NDR_record.float_rep) {
#if defined(__NDR_convert__float_rep__Reply__host_processor_info_t__out_processor_count__defined)
		__NDR_convert__float_rep__Reply__host_processor_info_t__out_processor_count(&Out0P->out_processor_count, Out0P->NDR.float_rep);
#endif /* __NDR_convert__float_rep__Reply__host_processor_info_t__out_processor_count__defined */
#if defined(__NDR_convert__float_rep__Reply__host_processor_info_t__out_processor_info__defined)
		__NDR_convert__float_rep__Reply__host_processor_info_t__out_processor_info((processor_info_array_t)(Out0P->out_processor_info.address), Out0P->NDR.float_rep, Out0P->out_processor_infoCnt);
#endif /* __NDR_convert__float_rep__Reply__host_processor_info_t__out_processor_info__defined */
	}
#endif	/* defined(__NDR_convert__float_rep...) */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Reply__host_processor_info_t__defined) */
#endif /* __MIG_check__Reply__mach_host_subsystem__ */
#endif /* ( __MigTypeCheck || __NDR_convert__ ) */


/* Routine host_processor_info */
mig_external kern_return_t host_processor_info
(
	host_t host,
	processor_flavor_t flavor,
	natural_t *out_processor_count,
	processor_info_array_t *out_processor_info,
	mach_msg_type_number_t *out_processor_infoCnt
)
{

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		NDR_record_t NDR;
		processor_flavor_t flavor;
	} Request;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		mach_msg_ool_descriptor_t out_processor_info;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		natural_t out_processor_count;
		mach_msg_type_number_t out_processor_infoCnt;
		mach_msg_trailer_t trailer;
	} Reply;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		mach_msg_ool_descriptor_t out_processor_info;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		natural_t out_processor_count;
		mach_msg_type_number_t out_processor_infoCnt;
	} __Reply;
#ifdef  __MigPackStructs
#pragma pack()
#endif
	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	union {
		Request In;
		Reply Out;
	} Mess;

	Request *InP = &Mess.In;
	Reply *Out0P = &Mess.Out;

	mach_msg_return_t msg_result;

#ifdef	__MIG_check__Reply__host_processor_info_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Reply__host_processor_info_t__defined */

	__DeclareSendRpc(204, "host_processor_info")

	InP->NDR = NDR_record;

	InP->flavor = flavor;

	InP->Head.msgh_bits =
		MACH_MSGH_BITS(19, MACH_MSG_TYPE_MAKE_SEND_ONCE);
	/* msgh_size passed as argument */
	InP->Head.msgh_request_port = host;
	InP->Head.msgh_reply_port = mig_get_reply_port();
	InP->Head.msgh_id = 204;

	__BeforeSendRpc(204, "host_processor_info")
	msg_result = mach_msg(&InP->Head, MACH_SEND_MSG|MACH_RCV_MSG|MACH_MSG_OPTION_NONE, (mach_msg_size_t)sizeof(Request), (mach_msg_size_t)sizeof(Reply), InP->Head.msgh_reply_port, MACH_MSG_TIMEOUT_NONE, MACH_PORT_NULL);
	__AfterSendRpc(204, "host_processor_info")
	if (msg_result != MACH_MSG_SUCCESS) {
		__MachMsgErrorWithoutTimeout(msg_result);
		{ return msg_result; }
	}


#if	defined(__MIG_check__Reply__host_processor_info_t__defined)
	check_result = __MIG_check__Reply__host_processor_info_t((__Reply__host_processor_info_t *)Out0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ return check_result; }
#endif	/* defined(__MIG_check__Reply__host_processor_info_t__defined) */

	*out_processor_count = Out0P->out_processor_count;

	*out_processor_info = (processor_info_array_t)(Out0P->out_processor_info.address);
	*out_processor_infoCnt = Out0P->out_processor_infoCnt;

	return KERN_SUCCESS;
}

#if ( __MigTypeCheck || __NDR_convert__ )
#if __MIG_check__Reply__mach_host_subsystem__
#if !defined(__MIG_check__Reply__host_get_io_master_t__defined)
#define __MIG_check__Reply__host_get_io_master_t__defined

mig_internal kern_return_t __MIG_check__Reply__host_get_io_master_t(__Reply__host_get_io_master_t *Out0P)
{

	typedef __Reply__host_get_io_master_t __Reply;
	boolean_t msgh_simple;
#if	__MigTypeCheck
	unsigned int msgh_size;
#endif	/* __MigTypeCheck */
	if (Out0P->Head.msgh_id != 305) {
	    if (Out0P->Head.msgh_id == MACH_NOTIFY_SEND_ONCE)
		{ return MIG_SERVER_DIED; }
	    else
		{ return MIG_REPLY_MISMATCH; }
	}

	msgh_simple = !(Out0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX);
#if	__MigTypeCheck
	msgh_size = Out0P->Head.msgh_size;

	if ((msgh_simple || Out0P->msgh_body.msgh_descriptor_count != 1 ||
	    msgh_size != (mach_msg_size_t)sizeof(__Reply)) &&
	    (!msgh_simple || msgh_size != (mach_msg_size_t)sizeof(mig_reply_error_t) ||
	    ((mig_reply_error_t *)Out0P)->RetCode == KERN_SUCCESS))
		{ return MIG_TYPE_ERROR ; }
#endif	/* __MigTypeCheck */

	if (msgh_simple) {
#ifdef	__NDR_convert__mig_reply_error_t__defined
		__NDR_convert__mig_reply_error_t((mig_reply_error_t *)Out0P);
#endif	/* __NDR_convert__mig_reply_error_t__defined */
		return ((mig_reply_error_t *)Out0P)->RetCode;
	}

#if	__MigTypeCheck
	if (Out0P->io_master.type != MACH_MSG_PORT_DESCRIPTOR ||
	    Out0P->io_master.disposition != 17) {
		return MIG_TYPE_ERROR;
	}
#endif	/* __MigTypeCheck */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Reply__host_get_io_master_t__defined) */
#endif /* __MIG_check__Reply__mach_host_subsystem__ */
#endif /* ( __MigTypeCheck || __NDR_convert__ ) */


/* Routine host_get_io_master */
mig_external kern_return_t host_get_io_master
(
	host_t host,
	io_master_t *io_master
)
{

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
	} Request;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		mach_msg_port_descriptor_t io_master;
		/* end of the kernel processed data */
		mach_msg_trailer_t trailer;
	} Reply;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		mach_msg_port_descriptor_t io_master;
		/* end of the kernel processed data */
	} __Reply;
#ifdef  __MigPackStructs
#pragma pack()
#endif
	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	union {
		Request In;
		Reply Out;
	} Mess;

	Request *InP = &Mess.In;
	Reply *Out0P = &Mess.Out;

	mach_msg_return_t msg_result;

#ifdef	__MIG_check__Reply__host_get_io_master_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Reply__host_get_io_master_t__defined */

	__DeclareSendRpc(205, "host_get_io_master")

	InP->Head.msgh_bits =
		MACH_MSGH_BITS(19, MACH_MSG_TYPE_MAKE_SEND_ONCE);
	/* msgh_size passed as argument */
	InP->Head.msgh_request_port = host;
	InP->Head.msgh_reply_port = mig_get_reply_port();
	InP->Head.msgh_id = 205;

	__BeforeSendRpc(205, "host_get_io_master")
	msg_result = mach_msg(&InP->Head, MACH_SEND_MSG|MACH_RCV_MSG|MACH_MSG_OPTION_NONE, (mach_msg_size_t)sizeof(Request), (mach_msg_size_t)sizeof(Reply), InP->Head.msgh_reply_port, MACH_MSG_TIMEOUT_NONE, MACH_PORT_NULL);
	__AfterSendRpc(205, "host_get_io_master")
	if (msg_result != MACH_MSG_SUCCESS) {
		__MachMsgErrorWithoutTimeout(msg_result);
		{ return msg_result; }
	}


#if	defined(__MIG_check__Reply__host_get_io_master_t__defined)
	check_result = __MIG_check__Reply__host_get_io_master_t((__Reply__host_get_io_master_t *)Out0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ return check_result; }
#endif	/* defined(__MIG_check__Reply__host_get_io_master_t__defined) */

	*io_master = Out0P->io_master.name;
	return KERN_SUCCESS;
}

#if ( __MigTypeCheck || __NDR_convert__ )
#if __MIG_check__Reply__mach_host_subsystem__
#if !defined(__MIG_check__Reply__host_get_clock_service_t__defined)
#define __MIG_check__Reply__host_get_clock_service_t__defined

mig_internal kern_return_t __MIG_check__Reply__host_get_clock_service_t(__Reply__host_get_clock_service_t *Out0P)
{

	typedef __Reply__host_get_clock_service_t __Reply;
	boolean_t msgh_simple;
#if	__MigTypeCheck
	unsigned int msgh_size;
#endif	/* __MigTypeCheck */
	if (Out0P->Head.msgh_id != 306) {
	    if (Out0P->Head.msgh_id == MACH_NOTIFY_SEND_ONCE)
		{ return MIG_SERVER_DIED; }
	    else
		{ return MIG_REPLY_MISMATCH; }
	}

	msgh_simple = !(Out0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX);
#if	__MigTypeCheck
	msgh_size = Out0P->Head.msgh_size;

	if ((msgh_simple || Out0P->msgh_body.msgh_descriptor_count != 1 ||
	    msgh_size != (mach_msg_size_t)sizeof(__Reply)) &&
	    (!msgh_simple || msgh_size != (mach_msg_size_t)sizeof(mig_reply_error_t) ||
	    ((mig_reply_error_t *)Out0P)->RetCode == KERN_SUCCESS))
		{ return MIG_TYPE_ERROR ; }
#endif	/* __MigTypeCheck */

	if (msgh_simple) {
#ifdef	__NDR_convert__mig_reply_error_t__defined
		__NDR_convert__mig_reply_error_t((mig_reply_error_t *)Out0P);
#endif	/* __NDR_convert__mig_reply_error_t__defined */
		return ((mig_reply_error_t *)Out0P)->RetCode;
	}

#if	__MigTypeCheck
	if (Out0P->clock_serv.type != MACH_MSG_PORT_DESCRIPTOR ||
	    Out0P->clock_serv.disposition != 17) {
		return MIG_TYPE_ERROR;
	}
#endif	/* __MigTypeCheck */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Reply__host_get_clock_service_t__defined) */
#endif /* __MIG_check__Reply__mach_host_subsystem__ */
#endif /* ( __MigTypeCheck || __NDR_convert__ ) */


/* Routine host_get_clock_service */
mig_external kern_return_t host_get_clock_service
(
	host_t host,
	clock_id_t clock_id,
	clock_serv_t *clock_serv
)
{

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		NDR_record_t NDR;
		clock_id_t clock_id;
	} Request;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		mach_msg_port_descriptor_t clock_serv;
		/* end of the kernel processed data */
		mach_msg_trailer_t trailer;
	} Reply;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		mach_msg_port_descriptor_t clock_serv;
		/* end of the kernel processed data */
	} __Reply;
#ifdef  __MigPackStructs
#pragma pack()
#endif
	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	union {
		Request In;
		Reply Out;
	} Mess;

	Request *InP = &Mess.In;
	Reply *Out0P = &Mess.Out;

	mach_msg_return_t msg_result;

#ifdef	__MIG_check__Reply__host_get_clock_service_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Reply__host_get_clock_service_t__defined */

	__DeclareSendRpc(206, "host_get_clock_service")

	InP->NDR = NDR_record;

	InP->clock_id = clock_id;

	InP->Head.msgh_bits =
		MACH_MSGH_BITS(19, MACH_MSG_TYPE_MAKE_SEND_ONCE);
	/* msgh_size passed as argument */
	InP->Head.msgh_request_port = host;
	InP->Head.msgh_reply_port = mig_get_reply_port();
	InP->Head.msgh_id = 206;

	__BeforeSendRpc(206, "host_get_clock_service")
	msg_result = mach_msg(&InP->Head, MACH_SEND_MSG|MACH_RCV_MSG|MACH_MSG_OPTION_NONE, (mach_msg_size_t)sizeof(Request), (mach_msg_size_t)sizeof(Reply), InP->Head.msgh_reply_port, MACH_MSG_TIMEOUT_NONE, MACH_PORT_NULL);
	__AfterSendRpc(206, "host_get_clock_service")
	if (msg_result != MACH_MSG_SUCCESS) {
		__MachMsgErrorWithoutTimeout(msg_result);
		{ return msg_result; }
	}


#if	defined(__MIG_check__Reply__host_get_clock_service_t__defined)
	check_result = __MIG_check__Reply__host_get_clock_service_t((__Reply__host_get_clock_service_t *)Out0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ return check_result; }
#endif	/* defined(__MIG_check__Reply__host_get_clock_service_t__defined) */

	*clock_serv = Out0P->clock_serv.name;
	return KERN_SUCCESS;
}

#if ( __MigTypeCheck || __NDR_convert__ )
#if __MIG_check__Reply__mach_host_subsystem__
#if !defined(__MIG_check__Reply__kmod_get_info_t__defined)
#define __MIG_check__Reply__kmod_get_info_t__defined
#ifndef __NDR_convert__int_rep__Reply__kmod_get_info_t__modules__defined
#if	defined(__NDR_convert__int_rep__mach_host__kmod_args_t__defined)
#define	__NDR_convert__int_rep__Reply__kmod_get_info_t__modules__defined
#define	__NDR_convert__int_rep__Reply__kmod_get_info_t__modules(a, f, c) \
	__NDR_convert__int_rep__mach_host__kmod_args_t((kmod_args_t *)(a), f, c)
#elif	defined(__NDR_convert__int_rep__kmod_args_t__defined)
#define	__NDR_convert__int_rep__Reply__kmod_get_info_t__modules__defined
#define	__NDR_convert__int_rep__Reply__kmod_get_info_t__modules(a, f, c) \
	__NDR_convert__int_rep__kmod_args_t((kmod_args_t *)(a), f, c)
#endif /* defined(__NDR_convert__*__defined) */
#endif /* __NDR_convert__int_rep__Reply__kmod_get_info_t__modules__defined */


#ifndef __NDR_convert__int_rep__Reply__kmod_get_info_t__modulesCnt__defined
#if	defined(__NDR_convert__int_rep__mach_host__mach_msg_type_number_t__defined)
#define	__NDR_convert__int_rep__Reply__kmod_get_info_t__modulesCnt__defined
#define	__NDR_convert__int_rep__Reply__kmod_get_info_t__modulesCnt(a, f) \
	__NDR_convert__int_rep__mach_host__mach_msg_type_number_t((mach_msg_type_number_t *)(a), f)
#elif	defined(__NDR_convert__int_rep__mach_msg_type_number_t__defined)
#define	__NDR_convert__int_rep__Reply__kmod_get_info_t__modulesCnt__defined
#define	__NDR_convert__int_rep__Reply__kmod_get_info_t__modulesCnt(a, f) \
	__NDR_convert__int_rep__mach_msg_type_number_t((mach_msg_type_number_t *)(a), f)
#endif /* defined(__NDR_convert__*__defined) */
#endif /* __NDR_convert__int_rep__Reply__kmod_get_info_t__modulesCnt__defined */


#ifndef __NDR_convert__char_rep__Reply__kmod_get_info_t__modules__defined
#if	defined(__NDR_convert__char_rep__mach_host__kmod_args_t__defined)
#define	__NDR_convert__char_rep__Reply__kmod_get_info_t__modules__defined
#define	__NDR_convert__char_rep__Reply__kmod_get_info_t__modules(a, f, c) \
	__NDR_convert__char_rep__mach_host__kmod_args_t((kmod_args_t *)(a), f, c)
#elif	defined(__NDR_convert__char_rep__kmod_args_t__defined)
#define	__NDR_convert__char_rep__Reply__kmod_get_info_t__modules__defined
#define	__NDR_convert__char_rep__Reply__kmod_get_info_t__modules(a, f, c) \
	__NDR_convert__char_rep__kmod_args_t((kmod_args_t *)(a), f, c)
#endif /* defined(__NDR_convert__*__defined) */
#endif /* __NDR_convert__char_rep__Reply__kmod_get_info_t__modules__defined */



#ifndef __NDR_convert__float_rep__Reply__kmod_get_info_t__modules__defined
#if	defined(__NDR_convert__float_rep__mach_host__kmod_args_t__defined)
#define	__NDR_convert__float_rep__Reply__kmod_get_info_t__modules__defined
#define	__NDR_convert__float_rep__Reply__kmod_get_info_t__modules(a, f, c) \
	__NDR_convert__float_rep__mach_host__kmod_args_t((kmod_args_t *)(a), f, c)
#elif	defined(__NDR_convert__float_rep__kmod_args_t__defined)
#define	__NDR_convert__float_rep__Reply__kmod_get_info_t__modules__defined
#define	__NDR_convert__float_rep__Reply__kmod_get_info_t__modules(a, f, c) \
	__NDR_convert__float_rep__kmod_args_t((kmod_args_t *)(a), f, c)
#endif /* defined(__NDR_convert__*__defined) */
#endif /* __NDR_convert__float_rep__Reply__kmod_get_info_t__modules__defined */




mig_internal kern_return_t __MIG_check__Reply__kmod_get_info_t(__Reply__kmod_get_info_t *Out0P)
{

	typedef __Reply__kmod_get_info_t __Reply;
	boolean_t msgh_simple;
#if	__MigTypeCheck
	unsigned int msgh_size;
#endif	/* __MigTypeCheck */
	if (Out0P->Head.msgh_id != 307) {
	    if (Out0P->Head.msgh_id == MACH_NOTIFY_SEND_ONCE)
		{ return MIG_SERVER_DIED; }
	    else
		{ return MIG_REPLY_MISMATCH; }
	}

	msgh_simple = !(Out0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX);
#if	__MigTypeCheck
	msgh_size = Out0P->Head.msgh_size;

	if ((msgh_simple || Out0P->msgh_body.msgh_descriptor_count != 1 ||
	    msgh_size != (mach_msg_size_t)sizeof(__Reply)) &&
	    (!msgh_simple || msgh_size != (mach_msg_size_t)sizeof(mig_reply_error_t) ||
	    ((mig_reply_error_t *)Out0P)->RetCode == KERN_SUCCESS))
		{ return MIG_TYPE_ERROR ; }
#endif	/* __MigTypeCheck */

	if (msgh_simple) {
#ifdef	__NDR_convert__mig_reply_error_t__defined
		__NDR_convert__mig_reply_error_t((mig_reply_error_t *)Out0P);
#endif	/* __NDR_convert__mig_reply_error_t__defined */
		return ((mig_reply_error_t *)Out0P)->RetCode;
	}

#if	__MigTypeCheck
	if (Out0P->modules.type != MACH_MSG_OOL_DESCRIPTOR) {
		return MIG_TYPE_ERROR;
	}
#endif	/* __MigTypeCheck */

#if	defined(__NDR_convert__int_rep__Reply__kmod_get_info_t__modules__defined) || \
	defined(__NDR_convert__int_rep__Reply__kmod_get_info_t__modulesCnt__defined)
	if (Out0P->NDR.int_rep != NDR_record.int_rep) {
#if defined(__NDR_convert__int_rep__Reply__kmod_get_info_t__modulesCnt__defined)
		__NDR_convert__int_rep__Reply__kmod_get_info_t__modulesCnt(&Out0P->modulesCnt, Out0P->NDR.int_rep);
#endif	/* __NDR_convert__int_rep__Reply__kmod_get_info_t__modulesCnt__defined */
#if defined(__NDR_convert__int_rep__Reply__kmod_get_info_t__modules__defined)
		__NDR_convert__int_rep__Reply__kmod_get_info_t__modules((kmod_args_t)(Out0P->modules.address), Out0P->NDR.int_rep, Out0P->modulesCnt);
#endif /* __NDR_convert__int_rep__Reply__kmod_get_info_t__modules__defined */
	}
#endif	/* defined(__NDR_convert__int_rep...) */

#if	defined(__NDR_convert__char_rep__Reply__kmod_get_info_t__modules__defined) || \
	0
	if (Out0P->NDR.char_rep != NDR_record.char_rep) {
#if defined(__NDR_convert__char_rep__Reply__kmod_get_info_t__modules__defined)
		__NDR_convert__char_rep__Reply__kmod_get_info_t__modules((kmod_args_t)(Out0P->modules.address), Out0P->NDR.char_rep, Out0P->modulesCnt);
#endif /* __NDR_convert__char_rep__Reply__kmod_get_info_t__modules__defined */
	}
#endif	/* defined(__NDR_convert__char_rep...) */

#if	defined(__NDR_convert__float_rep__Reply__kmod_get_info_t__modules__defined) || \
	0
	if (Out0P->NDR.float_rep != NDR_record.float_rep) {
#if defined(__NDR_convert__float_rep__Reply__kmod_get_info_t__modules__defined)
		__NDR_convert__float_rep__Reply__kmod_get_info_t__modules((kmod_args_t)(Out0P->modules.address), Out0P->NDR.float_rep, Out0P->modulesCnt);
#endif /* __NDR_convert__float_rep__Reply__kmod_get_info_t__modules__defined */
	}
#endif	/* defined(__NDR_convert__float_rep...) */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Reply__kmod_get_info_t__defined) */
#endif /* __MIG_check__Reply__mach_host_subsystem__ */
#endif /* ( __MigTypeCheck || __NDR_convert__ ) */


/* Routine kmod_get_info */
mig_external kern_return_t kmod_get_info
(
	host_t host,
	kmod_args_t *modules,
	mach_msg_type_number_t *modulesCnt
)
{

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
	} Request;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		mach_msg_ool_descriptor_t modules;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		mach_msg_type_number_t modulesCnt;
		mach_msg_trailer_t trailer;
	} Reply;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		mach_msg_ool_descriptor_t modules;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		mach_msg_type_number_t modulesCnt;
	} __Reply;
#ifdef  __MigPackStructs
#pragma pack()
#endif
	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	union {
		Request In;
		Reply Out;
	} Mess;

	Request *InP = &Mess.In;
	Reply *Out0P = &Mess.Out;

	mach_msg_return_t msg_result;

#ifdef	__MIG_check__Reply__kmod_get_info_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Reply__kmod_get_info_t__defined */

	__DeclareSendRpc(207, "kmod_get_info")

	InP->Head.msgh_bits =
		MACH_MSGH_BITS(19, MACH_MSG_TYPE_MAKE_SEND_ONCE);
	/* msgh_size passed as argument */
	InP->Head.msgh_request_port = host;
	InP->Head.msgh_reply_port = mig_get_reply_port();
	InP->Head.msgh_id = 207;

	__BeforeSendRpc(207, "kmod_get_info")
	msg_result = mach_msg(&InP->Head, MACH_SEND_MSG|MACH_RCV_MSG|MACH_MSG_OPTION_NONE, (mach_msg_size_t)sizeof(Request), (mach_msg_size_t)sizeof(Reply), InP->Head.msgh_reply_port, MACH_MSG_TIMEOUT_NONE, MACH_PORT_NULL);
	__AfterSendRpc(207, "kmod_get_info")
	if (msg_result != MACH_MSG_SUCCESS) {
		__MachMsgErrorWithoutTimeout(msg_result);
		{ return msg_result; }
	}


#if	defined(__MIG_check__Reply__kmod_get_info_t__defined)
	check_result = __MIG_check__Reply__kmod_get_info_t((__Reply__kmod_get_info_t *)Out0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ return check_result; }
#endif	/* defined(__MIG_check__Reply__kmod_get_info_t__defined) */

	*modules = (kmod_args_t)(Out0P->modules.address);
	*modulesCnt = Out0P->modulesCnt;

	return KERN_SUCCESS;
}

#if ( __MigTypeCheck || __NDR_convert__ )
#if __MIG_check__Reply__mach_host_subsystem__
#if !defined(__MIG_check__Reply__host_zone_info_t__defined)
#define __MIG_check__Reply__host_zone_info_t__defined
#ifndef __NDR_convert__int_rep__Reply__host_zone_info_t__names__defined
#if	defined(__NDR_convert__int_rep__mach_host__zone_name_array_t__defined)
#define	__NDR_convert__int_rep__Reply__host_zone_info_t__names__defined
#define	__NDR_convert__int_rep__Reply__host_zone_info_t__names(a, f, c) \
	__NDR_convert__int_rep__mach_host__zone_name_array_t((zone_name_array_t *)(a), f, c)
#elif	defined(__NDR_convert__int_rep__zone_name_array_t__defined)
#define	__NDR_convert__int_rep__Reply__host_zone_info_t__names__defined
#define	__NDR_convert__int_rep__Reply__host_zone_info_t__names(a, f, c) \
	__NDR_convert__int_rep__zone_name_array_t((zone_name_array_t *)(a), f, c)
#elif	defined(__NDR_convert__int_rep__mach_host__zone_name_t__defined)
#define	__NDR_convert__int_rep__Reply__host_zone_info_t__names__defined
#define	__NDR_convert__int_rep__Reply__host_zone_info_t__names(a, f, c) \
	__NDR_convert__ARRAY((zone_name_t *)(a), f, c, __NDR_convert__int_rep__mach_host__zone_name_t)
#elif	defined(__NDR_convert__int_rep__zone_name_t__defined)
#define	__NDR_convert__int_rep__Reply__host_zone_info_t__names__defined
#define	__NDR_convert__int_rep__Reply__host_zone_info_t__names(a, f, c) \
	__NDR_convert__ARRAY((zone_name_t *)(a), f, c, __NDR_convert__int_rep__zone_name_t)
#elif	defined(__NDR_convert__int_rep__mach_host__char__defined)
#define	__NDR_convert__int_rep__Reply__host_zone_info_t__names__defined
#define	__NDR_convert__int_rep__Reply__host_zone_info_t__names(a, f, c) \
	__NDR_convert__ARRAY((char *)(a), f, 80 * (c), __NDR_convert__int_rep__mach_host__char)
#elif	defined(__NDR_convert__int_rep__char__defined)
#define	__NDR_convert__int_rep__Reply__host_zone_info_t__names__defined
#define	__NDR_convert__int_rep__Reply__host_zone_info_t__names(a, f, c) \
	__NDR_convert__ARRAY((char *)(a), f, 80 * (c), __NDR_convert__int_rep__char)
#endif /* defined(__NDR_convert__*__defined) */
#endif /* __NDR_convert__int_rep__Reply__host_zone_info_t__names__defined */


#ifndef __NDR_convert__int_rep__Reply__host_zone_info_t__namesCnt__defined
#if	defined(__NDR_convert__int_rep__mach_host__mach_msg_type_number_t__defined)
#define	__NDR_convert__int_rep__Reply__host_zone_info_t__namesCnt__defined
#define	__NDR_convert__int_rep__Reply__host_zone_info_t__namesCnt(a, f) \
	__NDR_convert__int_rep__mach_host__mach_msg_type_number_t((mach_msg_type_number_t *)(a), f)
#elif	defined(__NDR_convert__int_rep__mach_msg_type_number_t__defined)
#define	__NDR_convert__int_rep__Reply__host_zone_info_t__namesCnt__defined
#define	__NDR_convert__int_rep__Reply__host_zone_info_t__namesCnt(a, f) \
	__NDR_convert__int_rep__mach_msg_type_number_t((mach_msg_type_number_t *)(a), f)
#endif /* defined(__NDR_convert__*__defined) */
#endif /* __NDR_convert__int_rep__Reply__host_zone_info_t__namesCnt__defined */


#ifndef __NDR_convert__int_rep__Reply__host_zone_info_t__info__defined
#if	defined(__NDR_convert__int_rep__mach_host__zone_info_array_t__defined)
#define	__NDR_convert__int_rep__Reply__host_zone_info_t__info__defined
#define	__NDR_convert__int_rep__Reply__host_zone_info_t__info(a, f, c) \
	__NDR_convert__int_rep__mach_host__zone_info_array_t((zone_info_array_t *)(a), f, c)
#elif	defined(__NDR_convert__int_rep__zone_info_array_t__defined)
#define	__NDR_convert__int_rep__Reply__host_zone_info_t__info__defined
#define	__NDR_convert__int_rep__Reply__host_zone_info_t__info(a, f, c) \
	__NDR_convert__int_rep__zone_info_array_t((zone_info_array_t *)(a), f, c)
#elif	defined(__NDR_convert__int_rep__mach_host__zone_info_t__defined)
#define	__NDR_convert__int_rep__Reply__host_zone_info_t__info__defined
#define	__NDR_convert__int_rep__Reply__host_zone_info_t__info(a, f, c) \
	__NDR_convert__ARRAY((zone_info_t *)(a), f, c, __NDR_convert__int_rep__mach_host__zone_info_t)
#elif	defined(__NDR_convert__int_rep__zone_info_t__defined)
#define	__NDR_convert__int_rep__Reply__host_zone_info_t__info__defined
#define	__NDR_convert__int_rep__Reply__host_zone_info_t__info(a, f, c) \
	__NDR_convert__ARRAY((zone_info_t *)(a), f, c, __NDR_convert__int_rep__zone_info_t)
#elif	defined(__NDR_convert__int_rep__mach_host__integer_t__defined)
#define	__NDR_convert__int_rep__Reply__host_zone_info_t__info__defined
#define	__NDR_convert__int_rep__Reply__host_zone_info_t__info(a, f, c) \
	__NDR_convert__ARRAY((integer_t *)(a), f, 9 * (c), __NDR_convert__int_rep__mach_host__integer_t)
#elif	defined(__NDR_convert__int_rep__integer_t__defined)
#define	__NDR_convert__int_rep__Reply__host_zone_info_t__info__defined
#define	__NDR_convert__int_rep__Reply__host_zone_info_t__info(a, f, c) \
	__NDR_convert__ARRAY((integer_t *)(a), f, 9 * (c), __NDR_convert__int_rep__integer_t)
#elif	defined(__NDR_convert__int_rep__mach_host__int32_t__defined)
#define	__NDR_convert__int_rep__Reply__host_zone_info_t__info__defined
#define	__NDR_convert__int_rep__Reply__host_zone_info_t__info(a, f, c) \
	__NDR_convert__ARRAY((int32_t *)(a), f, 9 * (c), __NDR_convert__int_rep__mach_host__int32_t)
#elif	defined(__NDR_convert__int_rep__int32_t__defined)
#define	__NDR_convert__int_rep__Reply__host_zone_info_t__info__defined
#define	__NDR_convert__int_rep__Reply__host_zone_info_t__info(a, f, c) \
	__NDR_convert__ARRAY((int32_t *)(a), f, 9 * (c), __NDR_convert__int_rep__int32_t)
#endif /* defined(__NDR_convert__*__defined) */
#endif /* __NDR_convert__int_rep__Reply__host_zone_info_t__info__defined */


#ifndef __NDR_convert__int_rep__Reply__host_zone_info_t__infoCnt__defined
#if	defined(__NDR_convert__int_rep__mach_host__mach_msg_type_number_t__defined)
#define	__NDR_convert__int_rep__Reply__host_zone_info_t__infoCnt__defined
#define	__NDR_convert__int_rep__Reply__host_zone_info_t__infoCnt(a, f) \
	__NDR_convert__int_rep__mach_host__mach_msg_type_number_t((mach_msg_type_number_t *)(a), f)
#elif	defined(__NDR_convert__int_rep__mach_msg_type_number_t__defined)
#define	__NDR_convert__int_rep__Reply__host_zone_info_t__infoCnt__defined
#define	__NDR_convert__int_rep__Reply__host_zone_info_t__infoCnt(a, f) \
	__NDR_convert__int_rep__mach_msg_type_number_t((mach_msg_type_number_t *)(a), f)
#endif /* defined(__NDR_convert__*__defined) */
#endif /* __NDR_convert__int_rep__Reply__host_zone_info_t__infoCnt__defined */


#ifndef __NDR_convert__char_rep__Reply__host_zone_info_t__names__defined
#if	defined(__NDR_convert__char_rep__mach_host__zone_name_array_t__defined)
#define	__NDR_convert__char_rep__Reply__host_zone_info_t__names__defined
#define	__NDR_convert__char_rep__Reply__host_zone_info_t__names(a, f, c) \
	__NDR_convert__char_rep__mach_host__zone_name_array_t((zone_name_array_t *)(a), f, c)
#elif	defined(__NDR_convert__char_rep__zone_name_array_t__defined)
#define	__NDR_convert__char_rep__Reply__host_zone_info_t__names__defined
#define	__NDR_convert__char_rep__Reply__host_zone_info_t__names(a, f, c) \
	__NDR_convert__char_rep__zone_name_array_t((zone_name_array_t *)(a), f, c)
#elif	defined(__NDR_convert__char_rep__mach_host__zone_name_t__defined)
#define	__NDR_convert__char_rep__Reply__host_zone_info_t__names__defined
#define	__NDR_convert__char_rep__Reply__host_zone_info_t__names(a, f, c) \
	__NDR_convert__ARRAY((zone_name_t *)(a), f, c, __NDR_convert__char_rep__mach_host__zone_name_t)
#elif	defined(__NDR_convert__char_rep__zone_name_t__defined)
#define	__NDR_convert__char_rep__Reply__host_zone_info_t__names__defined
#define	__NDR_convert__char_rep__Reply__host_zone_info_t__names(a, f, c) \
	__NDR_convert__ARRAY((zone_name_t *)(a), f, c, __NDR_convert__char_rep__zone_name_t)
#elif	defined(__NDR_convert__char_rep__mach_host__char__defined)
#define	__NDR_convert__char_rep__Reply__host_zone_info_t__names__defined
#define	__NDR_convert__char_rep__Reply__host_zone_info_t__names(a, f, c) \
	__NDR_convert__ARRAY((char *)(a), f, 80 * (c), __NDR_convert__char_rep__mach_host__char)
#elif	defined(__NDR_convert__char_rep__char__defined)
#define	__NDR_convert__char_rep__Reply__host_zone_info_t__names__defined
#define	__NDR_convert__char_rep__Reply__host_zone_info_t__names(a, f, c) \
	__NDR_convert__ARRAY((char *)(a), f, 80 * (c), __NDR_convert__char_rep__char)
#endif /* defined(__NDR_convert__*__defined) */
#endif /* __NDR_convert__char_rep__Reply__host_zone_info_t__names__defined */



#ifndef __NDR_convert__char_rep__Reply__host_zone_info_t__info__defined
#if	defined(__NDR_convert__char_rep__mach_host__zone_info_array_t__defined)
#define	__NDR_convert__char_rep__Reply__host_zone_info_t__info__defined
#define	__NDR_convert__char_rep__Reply__host_zone_info_t__info(a, f, c) \
	__NDR_convert__char_rep__mach_host__zone_info_array_t((zone_info_array_t *)(a), f, c)
#elif	defined(__NDR_convert__char_rep__zone_info_array_t__defined)
#define	__NDR_convert__char_rep__Reply__host_zone_info_t__info__defined
#define	__NDR_convert__char_rep__Reply__host_zone_info_t__info(a, f, c) \
	__NDR_convert__char_rep__zone_info_array_t((zone_info_array_t *)(a), f, c)
#elif	defined(__NDR_convert__char_rep__mach_host__zone_info_t__defined)
#define	__NDR_convert__char_rep__Reply__host_zone_info_t__info__defined
#define	__NDR_convert__char_rep__Reply__host_zone_info_t__info(a, f, c) \
	__NDR_convert__ARRAY((zone_info_t *)(a), f, c, __NDR_convert__char_rep__mach_host__zone_info_t)
#elif	defined(__NDR_convert__char_rep__zone_info_t__defined)
#define	__NDR_convert__char_rep__Reply__host_zone_info_t__info__defined
#define	__NDR_convert__char_rep__Reply__host_zone_info_t__info(a, f, c) \
	__NDR_convert__ARRAY((zone_info_t *)(a), f, c, __NDR_convert__char_rep__zone_info_t)
#elif	defined(__NDR_convert__char_rep__mach_host__integer_t__defined)
#define	__NDR_convert__char_rep__Reply__host_zone_info_t__info__defined
#define	__NDR_convert__char_rep__Reply__host_zone_info_t__info(a, f, c) \
	__NDR_convert__ARRAY((integer_t *)(a), f, 9 * (c), __NDR_convert__char_rep__mach_host__integer_t)
#elif	defined(__NDR_convert__char_rep__integer_t__defined)
#define	__NDR_convert__char_rep__Reply__host_zone_info_t__info__defined
#define	__NDR_convert__char_rep__Reply__host_zone_info_t__info(a, f, c) \
	__NDR_convert__ARRAY((integer_t *)(a), f, 9 * (c), __NDR_convert__char_rep__integer_t)
#elif	defined(__NDR_convert__char_rep__mach_host__int32_t__defined)
#define	__NDR_convert__char_rep__Reply__host_zone_info_t__info__defined
#define	__NDR_convert__char_rep__Reply__host_zone_info_t__info(a, f, c) \
	__NDR_convert__ARRAY((int32_t *)(a), f, 9 * (c), __NDR_convert__char_rep__mach_host__int32_t)
#elif	defined(__NDR_convert__char_rep__int32_t__defined)
#define	__NDR_convert__char_rep__Reply__host_zone_info_t__info__defined
#define	__NDR_convert__char_rep__Reply__host_zone_info_t__info(a, f, c) \
	__NDR_convert__ARRAY((int32_t *)(a), f, 9 * (c), __NDR_convert__char_rep__int32_t)
#endif /* defined(__NDR_convert__*__defined) */
#endif /* __NDR_convert__char_rep__Reply__host_zone_info_t__info__defined */



#ifndef __NDR_convert__float_rep__Reply__host_zone_info_t__names__defined
#if	defined(__NDR_convert__float_rep__mach_host__zone_name_array_t__defined)
#define	__NDR_convert__float_rep__Reply__host_zone_info_t__names__defined
#define	__NDR_convert__float_rep__Reply__host_zone_info_t__names(a, f, c) \
	__NDR_convert__float_rep__mach_host__zone_name_array_t((zone_name_array_t *)(a), f, c)
#elif	defined(__NDR_convert__float_rep__zone_name_array_t__defined)
#define	__NDR_convert__float_rep__Reply__host_zone_info_t__names__defined
#define	__NDR_convert__float_rep__Reply__host_zone_info_t__names(a, f, c) \
	__NDR_convert__float_rep__zone_name_array_t((zone_name_array_t *)(a), f, c)
#elif	defined(__NDR_convert__float_rep__mach_host__zone_name_t__defined)
#define	__NDR_convert__float_rep__Reply__host_zone_info_t__names__defined
#define	__NDR_convert__float_rep__Reply__host_zone_info_t__names(a, f, c) \
	__NDR_convert__ARRAY((zone_name_t *)(a), f, c, __NDR_convert__float_rep__mach_host__zone_name_t)
#elif	defined(__NDR_convert__float_rep__zone_name_t__defined)
#define	__NDR_convert__float_rep__Reply__host_zone_info_t__names__defined
#define	__NDR_convert__float_rep__Reply__host_zone_info_t__names(a, f, c) \
	__NDR_convert__ARRAY((zone_name_t *)(a), f, c, __NDR_convert__float_rep__zone_name_t)
#elif	defined(__NDR_convert__float_rep__mach_host__char__defined)
#define	__NDR_convert__float_rep__Reply__host_zone_info_t__names__defined
#define	__NDR_convert__float_rep__Reply__host_zone_info_t__names(a, f, c) \
	__NDR_convert__ARRAY((char *)(a), f, 80 * (c), __NDR_convert__float_rep__mach_host__char)
#elif	defined(__NDR_convert__float_rep__char__defined)
#define	__NDR_convert__float_rep__Reply__host_zone_info_t__names__defined
#define	__NDR_convert__float_rep__Reply__host_zone_info_t__names(a, f, c) \
	__NDR_convert__ARRAY((char *)(a), f, 80 * (c), __NDR_convert__float_rep__char)
#endif /* defined(__NDR_convert__*__defined) */
#endif /* __NDR_convert__float_rep__Reply__host_zone_info_t__names__defined */



#ifndef __NDR_convert__float_rep__Reply__host_zone_info_t__info__defined
#if	defined(__NDR_convert__float_rep__mach_host__zone_info_array_t__defined)
#define	__NDR_convert__float_rep__Reply__host_zone_info_t__info__defined
#define	__NDR_convert__float_rep__Reply__host_zone_info_t__info(a, f, c) \
	__NDR_convert__float_rep__mach_host__zone_info_array_t((zone_info_array_t *)(a), f, c)
#elif	defined(__NDR_convert__float_rep__zone_info_array_t__defined)
#define	__NDR_convert__float_rep__Reply__host_zone_info_t__info__defined
#define	__NDR_convert__float_rep__Reply__host_zone_info_t__info(a, f, c) \
	__NDR_convert__float_rep__zone_info_array_t((zone_info_array_t *)(a), f, c)
#elif	defined(__NDR_convert__float_rep__mach_host__zone_info_t__defined)
#define	__NDR_convert__float_rep__Reply__host_zone_info_t__info__defined
#define	__NDR_convert__float_rep__Reply__host_zone_info_t__info(a, f, c) \
	__NDR_convert__ARRAY((zone_info_t *)(a), f, c, __NDR_convert__float_rep__mach_host__zone_info_t)
#elif	defined(__NDR_convert__float_rep__zone_info_t__defined)
#define	__NDR_convert__float_rep__Reply__host_zone_info_t__info__defined
#define	__NDR_convert__float_rep__Reply__host_zone_info_t__info(a, f, c) \
	__NDR_convert__ARRAY((zone_info_t *)(a), f, c, __NDR_convert__float_rep__zone_info_t)
#elif	defined(__NDR_convert__float_rep__mach_host__integer_t__defined)
#define	__NDR_convert__float_rep__Reply__host_zone_info_t__info__defined
#define	__NDR_convert__float_rep__Reply__host_zone_info_t__info(a, f, c) \
	__NDR_convert__ARRAY((integer_t *)(a), f, 9 * (c), __NDR_convert__float_rep__mach_host__integer_t)
#elif	defined(__NDR_convert__float_rep__integer_t__defined)
#define	__NDR_convert__float_rep__Reply__host_zone_info_t__info__defined
#define	__NDR_convert__float_rep__Reply__host_zone_info_t__info(a, f, c) \
	__NDR_convert__ARRAY((integer_t *)(a), f, 9 * (c), __NDR_convert__float_rep__integer_t)
#elif	defined(__NDR_convert__float_rep__mach_host__int32_t__defined)
#define	__NDR_convert__float_rep__Reply__host_zone_info_t__info__defined
#define	__NDR_convert__float_rep__Reply__host_zone_info_t__info(a, f, c) \
	__NDR_convert__ARRAY((int32_t *)(a), f, 9 * (c), __NDR_convert__float_rep__mach_host__int32_t)
#elif	defined(__NDR_convert__float_rep__int32_t__defined)
#define	__NDR_convert__float_rep__Reply__host_zone_info_t__info__defined
#define	__NDR_convert__float_rep__Reply__host_zone_info_t__info(a, f, c) \
	__NDR_convert__ARRAY((int32_t *)(a), f, 9 * (c), __NDR_convert__float_rep__int32_t)
#endif /* defined(__NDR_convert__*__defined) */
#endif /* __NDR_convert__float_rep__Reply__host_zone_info_t__info__defined */




mig_internal kern_return_t __MIG_check__Reply__host_zone_info_t(__Reply__host_zone_info_t *Out0P)
{

	typedef __Reply__host_zone_info_t __Reply;
	boolean_t msgh_simple;
#if	__MigTypeCheck
	unsigned int msgh_size;
#endif	/* __MigTypeCheck */
	if (Out0P->Head.msgh_id != 308) {
	    if (Out0P->Head.msgh_id == MACH_NOTIFY_SEND_ONCE)
		{ return MIG_SERVER_DIED; }
	    else
		{ return MIG_REPLY_MISMATCH; }
	}

	msgh_simple = !(Out0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX);
#if	__MigTypeCheck
	msgh_size = Out0P->Head.msgh_size;

	if ((msgh_simple || Out0P->msgh_body.msgh_descriptor_count != 2 ||
	    msgh_size != (mach_msg_size_t)sizeof(__Reply)) &&
	    (!msgh_simple || msgh_size != (mach_msg_size_t)sizeof(mig_reply_error_t) ||
	    ((mig_reply_error_t *)Out0P)->RetCode == KERN_SUCCESS))
		{ return MIG_TYPE_ERROR ; }
#endif	/* __MigTypeCheck */

	if (msgh_simple) {
#ifdef	__NDR_convert__mig_reply_error_t__defined
		__NDR_convert__mig_reply_error_t((mig_reply_error_t *)Out0P);
#endif	/* __NDR_convert__mig_reply_error_t__defined */
		return ((mig_reply_error_t *)Out0P)->RetCode;
	}

#if	__MigTypeCheck
	if (Out0P->names.type != MACH_MSG_OOL_DESCRIPTOR) {
		return MIG_TYPE_ERROR;
	}
#endif	/* __MigTypeCheck */

#if	__MigTypeCheck
	if (Out0P->info.type != MACH_MSG_OOL_DESCRIPTOR) {
		return MIG_TYPE_ERROR;
	}
#endif	/* __MigTypeCheck */

#if	defined(__NDR_convert__int_rep__Reply__host_zone_info_t__names__defined) || \
	defined(__NDR_convert__int_rep__Reply__host_zone_info_t__namesCnt__defined) || \
	defined(__NDR_convert__int_rep__Reply__host_zone_info_t__info__defined) || \
	defined(__NDR_convert__int_rep__Reply__host_zone_info_t__infoCnt__defined)
	if (Out0P->NDR.int_rep != NDR_record.int_rep) {
#if defined(__NDR_convert__int_rep__Reply__host_zone_info_t__namesCnt__defined)
		__NDR_convert__int_rep__Reply__host_zone_info_t__namesCnt(&Out0P->namesCnt, Out0P->NDR.int_rep);
#endif	/* __NDR_convert__int_rep__Reply__host_zone_info_t__namesCnt__defined */
#if defined(__NDR_convert__int_rep__Reply__host_zone_info_t__names__defined)
		__NDR_convert__int_rep__Reply__host_zone_info_t__names((zone_name_array_t)(Out0P->names.address), Out0P->NDR.int_rep, Out0P->namesCnt);
#endif /* __NDR_convert__int_rep__Reply__host_zone_info_t__names__defined */
#if defined(__NDR_convert__int_rep__Reply__host_zone_info_t__infoCnt__defined)
		__NDR_convert__int_rep__Reply__host_zone_info_t__infoCnt(&Out0P->infoCnt, Out0P->NDR.int_rep);
#endif	/* __NDR_convert__int_rep__Reply__host_zone_info_t__infoCnt__defined */
#if defined(__NDR_convert__int_rep__Reply__host_zone_info_t__info__defined)
		__NDR_convert__int_rep__Reply__host_zone_info_t__info((zone_info_array_t)(Out0P->info.address), Out0P->NDR.int_rep, Out0P->infoCnt);
#endif /* __NDR_convert__int_rep__Reply__host_zone_info_t__info__defined */
	}
#endif	/* defined(__NDR_convert__int_rep...) */

#if	defined(__NDR_convert__char_rep__Reply__host_zone_info_t__names__defined) || \
	0 || \
	defined(__NDR_convert__char_rep__Reply__host_zone_info_t__info__defined) || \
	0
	if (Out0P->NDR.char_rep != NDR_record.char_rep) {
#if defined(__NDR_convert__char_rep__Reply__host_zone_info_t__names__defined)
		__NDR_convert__char_rep__Reply__host_zone_info_t__names((zone_name_array_t)(Out0P->names.address), Out0P->NDR.char_rep, Out0P->namesCnt);
#endif /* __NDR_convert__char_rep__Reply__host_zone_info_t__names__defined */
#if defined(__NDR_convert__char_rep__Reply__host_zone_info_t__info__defined)
		__NDR_convert__char_rep__Reply__host_zone_info_t__info((zone_info_array_t)(Out0P->info.address), Out0P->NDR.char_rep, Out0P->infoCnt);
#endif /* __NDR_convert__char_rep__Reply__host_zone_info_t__info__defined */
	}
#endif	/* defined(__NDR_convert__char_rep...) */

#if	defined(__NDR_convert__float_rep__Reply__host_zone_info_t__names__defined) || \
	0 || \
	defined(__NDR_convert__float_rep__Reply__host_zone_info_t__info__defined) || \
	0
	if (Out0P->NDR.float_rep != NDR_record.float_rep) {
#if defined(__NDR_convert__float_rep__Reply__host_zone_info_t__names__defined)
		__NDR_convert__float_rep__Reply__host_zone_info_t__names((zone_name_array_t)(Out0P->names.address), Out0P->NDR.float_rep, Out0P->namesCnt);
#endif /* __NDR_convert__float_rep__Reply__host_zone_info_t__names__defined */
#if defined(__NDR_convert__float_rep__Reply__host_zone_info_t__info__defined)
		__NDR_convert__float_rep__Reply__host_zone_info_t__info((zone_info_array_t)(Out0P->info.address), Out0P->NDR.float_rep, Out0P->infoCnt);
#endif /* __NDR_convert__float_rep__Reply__host_zone_info_t__info__defined */
	}
#endif	/* defined(__NDR_convert__float_rep...) */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Reply__host_zone_info_t__defined) */
#endif /* __MIG_check__Reply__mach_host_subsystem__ */
#endif /* ( __MigTypeCheck || __NDR_convert__ ) */


/* Routine host_zone_info */
mig_external kern_return_t host_zone_info
(
	host_t host,
	zone_name_array_t *names,
	mach_msg_type_number_t *namesCnt,
	zone_info_array_t *info,
	mach_msg_type_number_t *infoCnt
)
{

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
	} Request;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		mach_msg_ool_descriptor_t names;
		mach_msg_ool_descriptor_t info;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		mach_msg_type_number_t namesCnt;
		mach_msg_type_number_t infoCnt;
		mach_msg_trailer_t trailer;
	} Reply;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		mach_msg_ool_descriptor_t names;
		mach_msg_ool_descriptor_t info;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		mach_msg_type_number_t namesCnt;
		mach_msg_type_number_t infoCnt;
	} __Reply;
#ifdef  __MigPackStructs
#pragma pack()
#endif
	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	union {
		Request In;
		Reply Out;
	} Mess;

	Request *InP = &Mess.In;
	Reply *Out0P = &Mess.Out;

	mach_msg_return_t msg_result;

#ifdef	__MIG_check__Reply__host_zone_info_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Reply__host_zone_info_t__defined */

	__DeclareSendRpc(208, "host_zone_info")

	InP->Head.msgh_bits =
		MACH_MSGH_BITS(19, MACH_MSG_TYPE_MAKE_SEND_ONCE);
	/* msgh_size passed as argument */
	InP->Head.msgh_request_port = host;
	InP->Head.msgh_reply_port = mig_get_reply_port();
	InP->Head.msgh_id = 208;

	__BeforeSendRpc(208, "host_zone_info")
	msg_result = mach_msg(&InP->Head, MACH_SEND_MSG|MACH_RCV_MSG|MACH_MSG_OPTION_NONE, (mach_msg_size_t)sizeof(Request), (mach_msg_size_t)sizeof(Reply), InP->Head.msgh_reply_port, MACH_MSG_TIMEOUT_NONE, MACH_PORT_NULL);
	__AfterSendRpc(208, "host_zone_info")
	if (msg_result != MACH_MSG_SUCCESS) {
		__MachMsgErrorWithoutTimeout(msg_result);
		{ return msg_result; }
	}


#if	defined(__MIG_check__Reply__host_zone_info_t__defined)
	check_result = __MIG_check__Reply__host_zone_info_t((__Reply__host_zone_info_t *)Out0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ return check_result; }
#endif	/* defined(__MIG_check__Reply__host_zone_info_t__defined) */

	*names = (zone_name_array_t)(Out0P->names.address);
	*namesCnt = Out0P->namesCnt;

	*info = (zone_info_array_t)(Out0P->info.address);
	*infoCnt = Out0P->infoCnt;

	return KERN_SUCCESS;
}

#if ( __MigTypeCheck || __NDR_convert__ )
#if __MIG_check__Reply__mach_host_subsystem__
#if !defined(__MIG_check__Reply__host_virtual_physical_table_info_t__defined)
#define __MIG_check__Reply__host_virtual_physical_table_info_t__defined
#ifndef __NDR_convert__int_rep__Reply__host_virtual_physical_table_info_t__info__defined
#if	defined(__NDR_convert__int_rep__mach_host__hash_info_bucket_array_t__defined)
#define	__NDR_convert__int_rep__Reply__host_virtual_physical_table_info_t__info__defined
#define	__NDR_convert__int_rep__Reply__host_virtual_physical_table_info_t__info(a, f, c) \
	__NDR_convert__int_rep__mach_host__hash_info_bucket_array_t((hash_info_bucket_array_t *)(a), f, c)
#elif	defined(__NDR_convert__int_rep__hash_info_bucket_array_t__defined)
#define	__NDR_convert__int_rep__Reply__host_virtual_physical_table_info_t__info__defined
#define	__NDR_convert__int_rep__Reply__host_virtual_physical_table_info_t__info(a, f, c) \
	__NDR_convert__int_rep__hash_info_bucket_array_t((hash_info_bucket_array_t *)(a), f, c)
#elif	defined(__NDR_convert__int_rep__mach_host__hash_info_bucket_t__defined)
#define	__NDR_convert__int_rep__Reply__host_virtual_physical_table_info_t__info__defined
#define	__NDR_convert__int_rep__Reply__host_virtual_physical_table_info_t__info(a, f, c) \
	__NDR_convert__ARRAY((hash_info_bucket_t *)(a), f, c, __NDR_convert__int_rep__mach_host__hash_info_bucket_t)
#elif	defined(__NDR_convert__int_rep__hash_info_bucket_t__defined)
#define	__NDR_convert__int_rep__Reply__host_virtual_physical_table_info_t__info__defined
#define	__NDR_convert__int_rep__Reply__host_virtual_physical_table_info_t__info(a, f, c) \
	__NDR_convert__ARRAY((hash_info_bucket_t *)(a), f, c, __NDR_convert__int_rep__hash_info_bucket_t)
#elif	defined(__NDR_convert__int_rep__mach_host__natural_t__defined)
#define	__NDR_convert__int_rep__Reply__host_virtual_physical_table_info_t__info__defined
#define	__NDR_convert__int_rep__Reply__host_virtual_physical_table_info_t__info(a, f, c) \
	__NDR_convert__ARRAY((natural_t *)(a), f, c, __NDR_convert__int_rep__mach_host__natural_t)
#elif	defined(__NDR_convert__int_rep__natural_t__defined)
#define	__NDR_convert__int_rep__Reply__host_virtual_physical_table_info_t__info__defined
#define	__NDR_convert__int_rep__Reply__host_virtual_physical_table_info_t__info(a, f, c) \
	__NDR_convert__ARRAY((natural_t *)(a), f, c, __NDR_convert__int_rep__natural_t)
#elif	defined(__NDR_convert__int_rep__mach_host__natural_t__defined)
#define	__NDR_convert__int_rep__Reply__host_virtual_physical_table_info_t__info__defined
#define	__NDR_convert__int_rep__Reply__host_virtual_physical_table_info_t__info(a, f, c) \
	__NDR_convert__ARRAY((natural_t *)(a), f, c, __NDR_convert__int_rep__mach_host__natural_t)
#elif	defined(__NDR_convert__int_rep__natural_t__defined)
#define	__NDR_convert__int_rep__Reply__host_virtual_physical_table_info_t__info__defined
#define	__NDR_convert__int_rep__Reply__host_virtual_physical_table_info_t__info(a, f, c) \
	__NDR_convert__ARRAY((natural_t *)(a), f, c, __NDR_convert__int_rep__natural_t)
#elif	defined(__NDR_convert__int_rep__mach_host__uint32_t__defined)
#define	__NDR_convert__int_rep__Reply__host_virtual_physical_table_info_t__info__defined
#define	__NDR_convert__int_rep__Reply__host_virtual_physical_table_info_t__info(a, f, c) \
	__NDR_convert__ARRAY((uint32_t *)(a), f, c, __NDR_convert__int_rep__mach_host__uint32_t)
#elif	defined(__NDR_convert__int_rep__uint32_t__defined)
#define	__NDR_convert__int_rep__Reply__host_virtual_physical_table_info_t__info__defined
#define	__NDR_convert__int_rep__Reply__host_virtual_physical_table_info_t__info(a, f, c) \
	__NDR_convert__ARRAY((uint32_t *)(a), f, c, __NDR_convert__int_rep__uint32_t)
#endif /* defined(__NDR_convert__*__defined) */
#endif /* __NDR_convert__int_rep__Reply__host_virtual_physical_table_info_t__info__defined */


#ifndef __NDR_convert__int_rep__Reply__host_virtual_physical_table_info_t__infoCnt__defined
#if	defined(__NDR_convert__int_rep__mach_host__mach_msg_type_number_t__defined)
#define	__NDR_convert__int_rep__Reply__host_virtual_physical_table_info_t__infoCnt__defined
#define	__NDR_convert__int_rep__Reply__host_virtual_physical_table_info_t__infoCnt(a, f) \
	__NDR_convert__int_rep__mach_host__mach_msg_type_number_t((mach_msg_type_number_t *)(a), f)
#elif	defined(__NDR_convert__int_rep__mach_msg_type_number_t__defined)
#define	__NDR_convert__int_rep__Reply__host_virtual_physical_table_info_t__infoCnt__defined
#define	__NDR_convert__int_rep__Reply__host_virtual_physical_table_info_t__infoCnt(a, f) \
	__NDR_convert__int_rep__mach_msg_type_number_t((mach_msg_type_number_t *)(a), f)
#endif /* defined(__NDR_convert__*__defined) */
#endif /* __NDR_convert__int_rep__Reply__host_virtual_physical_table_info_t__infoCnt__defined */


#ifndef __NDR_convert__char_rep__Reply__host_virtual_physical_table_info_t__info__defined
#if	defined(__NDR_convert__char_rep__mach_host__hash_info_bucket_array_t__defined)
#define	__NDR_convert__char_rep__Reply__host_virtual_physical_table_info_t__info__defined
#define	__NDR_convert__char_rep__Reply__host_virtual_physical_table_info_t__info(a, f, c) \
	__NDR_convert__char_rep__mach_host__hash_info_bucket_array_t((hash_info_bucket_array_t *)(a), f, c)
#elif	defined(__NDR_convert__char_rep__hash_info_bucket_array_t__defined)
#define	__NDR_convert__char_rep__Reply__host_virtual_physical_table_info_t__info__defined
#define	__NDR_convert__char_rep__Reply__host_virtual_physical_table_info_t__info(a, f, c) \
	__NDR_convert__char_rep__hash_info_bucket_array_t((hash_info_bucket_array_t *)(a), f, c)
#elif	defined(__NDR_convert__char_rep__mach_host__hash_info_bucket_t__defined)
#define	__NDR_convert__char_rep__Reply__host_virtual_physical_table_info_t__info__defined
#define	__NDR_convert__char_rep__Reply__host_virtual_physical_table_info_t__info(a, f, c) \
	__NDR_convert__ARRAY((hash_info_bucket_t *)(a), f, c, __NDR_convert__char_rep__mach_host__hash_info_bucket_t)
#elif	defined(__NDR_convert__char_rep__hash_info_bucket_t__defined)
#define	__NDR_convert__char_rep__Reply__host_virtual_physical_table_info_t__info__defined
#define	__NDR_convert__char_rep__Reply__host_virtual_physical_table_info_t__info(a, f, c) \
	__NDR_convert__ARRAY((hash_info_bucket_t *)(a), f, c, __NDR_convert__char_rep__hash_info_bucket_t)
#elif	defined(__NDR_convert__char_rep__mach_host__natural_t__defined)
#define	__NDR_convert__char_rep__Reply__host_virtual_physical_table_info_t__info__defined
#define	__NDR_convert__char_rep__Reply__host_virtual_physical_table_info_t__info(a, f, c) \
	__NDR_convert__ARRAY((natural_t *)(a), f, c, __NDR_convert__char_rep__mach_host__natural_t)
#elif	defined(__NDR_convert__char_rep__natural_t__defined)
#define	__NDR_convert__char_rep__Reply__host_virtual_physical_table_info_t__info__defined
#define	__NDR_convert__char_rep__Reply__host_virtual_physical_table_info_t__info(a, f, c) \
	__NDR_convert__ARRAY((natural_t *)(a), f, c, __NDR_convert__char_rep__natural_t)
#elif	defined(__NDR_convert__char_rep__mach_host__natural_t__defined)
#define	__NDR_convert__char_rep__Reply__host_virtual_physical_table_info_t__info__defined
#define	__NDR_convert__char_rep__Reply__host_virtual_physical_table_info_t__info(a, f, c) \
	__NDR_convert__ARRAY((natural_t *)(a), f, c, __NDR_convert__char_rep__mach_host__natural_t)
#elif	defined(__NDR_convert__char_rep__natural_t__defined)
#define	__NDR_convert__char_rep__Reply__host_virtual_physical_table_info_t__info__defined
#define	__NDR_convert__char_rep__Reply__host_virtual_physical_table_info_t__info(a, f, c) \
	__NDR_convert__ARRAY((natural_t *)(a), f, c, __NDR_convert__char_rep__natural_t)
#elif	defined(__NDR_convert__char_rep__mach_host__uint32_t__defined)
#define	__NDR_convert__char_rep__Reply__host_virtual_physical_table_info_t__info__defined
#define	__NDR_convert__char_rep__Reply__host_virtual_physical_table_info_t__info(a, f, c) \
	__NDR_convert__ARRAY((uint32_t *)(a), f, c, __NDR_convert__char_rep__mach_host__uint32_t)
#elif	defined(__NDR_convert__char_rep__uint32_t__defined)
#define	__NDR_convert__char_rep__Reply__host_virtual_physical_table_info_t__info__defined
#define	__NDR_convert__char_rep__Reply__host_virtual_physical_table_info_t__info(a, f, c) \
	__NDR_convert__ARRAY((uint32_t *)(a), f, c, __NDR_convert__char_rep__uint32_t)
#endif /* defined(__NDR_convert__*__defined) */
#endif /* __NDR_convert__char_rep__Reply__host_virtual_physical_table_info_t__info__defined */



#ifndef __NDR_convert__float_rep__Reply__host_virtual_physical_table_info_t__info__defined
#if	defined(__NDR_convert__float_rep__mach_host__hash_info_bucket_array_t__defined)
#define	__NDR_convert__float_rep__Reply__host_virtual_physical_table_info_t__info__defined
#define	__NDR_convert__float_rep__Reply__host_virtual_physical_table_info_t__info(a, f, c) \
	__NDR_convert__float_rep__mach_host__hash_info_bucket_array_t((hash_info_bucket_array_t *)(a), f, c)
#elif	defined(__NDR_convert__float_rep__hash_info_bucket_array_t__defined)
#define	__NDR_convert__float_rep__Reply__host_virtual_physical_table_info_t__info__defined
#define	__NDR_convert__float_rep__Reply__host_virtual_physical_table_info_t__info(a, f, c) \
	__NDR_convert__float_rep__hash_info_bucket_array_t((hash_info_bucket_array_t *)(a), f, c)
#elif	defined(__NDR_convert__float_rep__mach_host__hash_info_bucket_t__defined)
#define	__NDR_convert__float_rep__Reply__host_virtual_physical_table_info_t__info__defined
#define	__NDR_convert__float_rep__Reply__host_virtual_physical_table_info_t__info(a, f, c) \
	__NDR_convert__ARRAY((hash_info_bucket_t *)(a), f, c, __NDR_convert__float_rep__mach_host__hash_info_bucket_t)
#elif	defined(__NDR_convert__float_rep__hash_info_bucket_t__defined)
#define	__NDR_convert__float_rep__Reply__host_virtual_physical_table_info_t__info__defined
#define	__NDR_convert__float_rep__Reply__host_virtual_physical_table_info_t__info(a, f, c) \
	__NDR_convert__ARRAY((hash_info_bucket_t *)(a), f, c, __NDR_convert__float_rep__hash_info_bucket_t)
#elif	defined(__NDR_convert__float_rep__mach_host__natural_t__defined)
#define	__NDR_convert__float_rep__Reply__host_virtual_physical_table_info_t__info__defined
#define	__NDR_convert__float_rep__Reply__host_virtual_physical_table_info_t__info(a, f, c) \
	__NDR_convert__ARRAY((natural_t *)(a), f, c, __NDR_convert__float_rep__mach_host__natural_t)
#elif	defined(__NDR_convert__float_rep__natural_t__defined)
#define	__NDR_convert__float_rep__Reply__host_virtual_physical_table_info_t__info__defined
#define	__NDR_convert__float_rep__Reply__host_virtual_physical_table_info_t__info(a, f, c) \
	__NDR_convert__ARRAY((natural_t *)(a), f, c, __NDR_convert__float_rep__natural_t)
#elif	defined(__NDR_convert__float_rep__mach_host__natural_t__defined)
#define	__NDR_convert__float_rep__Reply__host_virtual_physical_table_info_t__info__defined
#define	__NDR_convert__float_rep__Reply__host_virtual_physical_table_info_t__info(a, f, c) \
	__NDR_convert__ARRAY((natural_t *)(a), f, c, __NDR_convert__float_rep__mach_host__natural_t)
#elif	defined(__NDR_convert__float_rep__natural_t__defined)
#define	__NDR_convert__float_rep__Reply__host_virtual_physical_table_info_t__info__defined
#define	__NDR_convert__float_rep__Reply__host_virtual_physical_table_info_t__info(a, f, c) \
	__NDR_convert__ARRAY((natural_t *)(a), f, c, __NDR_convert__float_rep__natural_t)
#elif	defined(__NDR_convert__float_rep__mach_host__uint32_t__defined)
#define	__NDR_convert__float_rep__Reply__host_virtual_physical_table_info_t__info__defined
#define	__NDR_convert__float_rep__Reply__host_virtual_physical_table_info_t__info(a, f, c) \
	__NDR_convert__ARRAY((uint32_t *)(a), f, c, __NDR_convert__float_rep__mach_host__uint32_t)
#elif	defined(__NDR_convert__float_rep__uint32_t__defined)
#define	__NDR_convert__float_rep__Reply__host_virtual_physical_table_info_t__info__defined
#define	__NDR_convert__float_rep__Reply__host_virtual_physical_table_info_t__info(a, f, c) \
	__NDR_convert__ARRAY((uint32_t *)(a), f, c, __NDR_convert__float_rep__uint32_t)
#endif /* defined(__NDR_convert__*__defined) */
#endif /* __NDR_convert__float_rep__Reply__host_virtual_physical_table_info_t__info__defined */




mig_internal kern_return_t __MIG_check__Reply__host_virtual_physical_table_info_t(__Reply__host_virtual_physical_table_info_t *Out0P)
{

	typedef __Reply__host_virtual_physical_table_info_t __Reply;
	boolean_t msgh_simple;
#if	__MigTypeCheck
	unsigned int msgh_size;
#endif	/* __MigTypeCheck */
	if (Out0P->Head.msgh_id != 309) {
	    if (Out0P->Head.msgh_id == MACH_NOTIFY_SEND_ONCE)
		{ return MIG_SERVER_DIED; }
	    else
		{ return MIG_REPLY_MISMATCH; }
	}

	msgh_simple = !(Out0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX);
#if	__MigTypeCheck
	msgh_size = Out0P->Head.msgh_size;

	if ((msgh_simple || Out0P->msgh_body.msgh_descriptor_count != 1 ||
	    msgh_size != (mach_msg_size_t)sizeof(__Reply)) &&
	    (!msgh_simple || msgh_size != (mach_msg_size_t)sizeof(mig_reply_error_t) ||
	    ((mig_reply_error_t *)Out0P)->RetCode == KERN_SUCCESS))
		{ return MIG_TYPE_ERROR ; }
#endif	/* __MigTypeCheck */

	if (msgh_simple) {
#ifdef	__NDR_convert__mig_reply_error_t__defined
		__NDR_convert__mig_reply_error_t((mig_reply_error_t *)Out0P);
#endif	/* __NDR_convert__mig_reply_error_t__defined */
		return ((mig_reply_error_t *)Out0P)->RetCode;
	}

#if	__MigTypeCheck
	if (Out0P->info.type != MACH_MSG_OOL_DESCRIPTOR) {
		return MIG_TYPE_ERROR;
	}
#endif	/* __MigTypeCheck */

#if	defined(__NDR_convert__int_rep__Reply__host_virtual_physical_table_info_t__info__defined) || \
	defined(__NDR_convert__int_rep__Reply__host_virtual_physical_table_info_t__infoCnt__defined)
	if (Out0P->NDR.int_rep != NDR_record.int_rep) {
#if defined(__NDR_convert__int_rep__Reply__host_virtual_physical_table_info_t__infoCnt__defined)
		__NDR_convert__int_rep__Reply__host_virtual_physical_table_info_t__infoCnt(&Out0P->infoCnt, Out0P->NDR.int_rep);
#endif	/* __NDR_convert__int_rep__Reply__host_virtual_physical_table_info_t__infoCnt__defined */
#if defined(__NDR_convert__int_rep__Reply__host_virtual_physical_table_info_t__info__defined)
		__NDR_convert__int_rep__Reply__host_virtual_physical_table_info_t__info((hash_info_bucket_array_t)(Out0P->info.address), Out0P->NDR.int_rep, Out0P->infoCnt);
#endif /* __NDR_convert__int_rep__Reply__host_virtual_physical_table_info_t__info__defined */
	}
#endif	/* defined(__NDR_convert__int_rep...) */

#if	defined(__NDR_convert__char_rep__Reply__host_virtual_physical_table_info_t__info__defined) || \
	0
	if (Out0P->NDR.char_rep != NDR_record.char_rep) {
#if defined(__NDR_convert__char_rep__Reply__host_virtual_physical_table_info_t__info__defined)
		__NDR_convert__char_rep__Reply__host_virtual_physical_table_info_t__info((hash_info_bucket_array_t)(Out0P->info.address), Out0P->NDR.char_rep, Out0P->infoCnt);
#endif /* __NDR_convert__char_rep__Reply__host_virtual_physical_table_info_t__info__defined */
	}
#endif	/* defined(__NDR_convert__char_rep...) */

#if	defined(__NDR_convert__float_rep__Reply__host_virtual_physical_table_info_t__info__defined) || \
	0
	if (Out0P->NDR.float_rep != NDR_record.float_rep) {
#if defined(__NDR_convert__float_rep__Reply__host_virtual_physical_table_info_t__info__defined)
		__NDR_convert__float_rep__Reply__host_virtual_physical_table_info_t__info((hash_info_bucket_array_t)(Out0P->info.address), Out0P->NDR.float_rep, Out0P->infoCnt);
#endif /* __NDR_convert__float_rep__Reply__host_virtual_physical_table_info_t__info__defined */
	}
#endif	/* defined(__NDR_convert__float_rep...) */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Reply__host_virtual_physical_table_info_t__defined) */
#endif /* __MIG_check__Reply__mach_host_subsystem__ */
#endif /* ( __MigTypeCheck || __NDR_convert__ ) */


/* Routine host_virtual_physical_table_info */
mig_external kern_return_t host_virtual_physical_table_info
(
	host_t host,
	hash_info_bucket_array_t *info,
	mach_msg_type_number_t *infoCnt
)
{

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
	} Request;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		mach_msg_ool_descriptor_t info;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		mach_msg_type_number_t infoCnt;
		mach_msg_trailer_t trailer;
	} Reply;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		mach_msg_ool_descriptor_t info;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		mach_msg_type_number_t infoCnt;
	} __Reply;
#ifdef  __MigPackStructs
#pragma pack()
#endif
	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	union {
		Request In;
		Reply Out;
	} Mess;

	Request *InP = &Mess.In;
	Reply *Out0P = &Mess.Out;

	mach_msg_return_t msg_result;

#ifdef	__MIG_check__Reply__host_virtual_physical_table_info_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Reply__host_virtual_physical_table_info_t__defined */

	__DeclareSendRpc(209, "host_virtual_physical_table_info")

	InP->Head.msgh_bits =
		MACH_MSGH_BITS(19, MACH_MSG_TYPE_MAKE_SEND_ONCE);
	/* msgh_size passed as argument */
	InP->Head.msgh_request_port = host;
	InP->Head.msgh_reply_port = mig_get_reply_port();
	InP->Head.msgh_id = 209;

	__BeforeSendRpc(209, "host_virtual_physical_table_info")
	msg_result = mach_msg(&InP->Head, MACH_SEND_MSG|MACH_RCV_MSG|MACH_MSG_OPTION_NONE, (mach_msg_size_t)sizeof(Request), (mach_msg_size_t)sizeof(Reply), InP->Head.msgh_reply_port, MACH_MSG_TIMEOUT_NONE, MACH_PORT_NULL);
	__AfterSendRpc(209, "host_virtual_physical_table_info")
	if (msg_result != MACH_MSG_SUCCESS) {
		__MachMsgErrorWithoutTimeout(msg_result);
		{ return msg_result; }
	}


#if	defined(__MIG_check__Reply__host_virtual_physical_table_info_t__defined)
	check_result = __MIG_check__Reply__host_virtual_physical_table_info_t((__Reply__host_virtual_physical_table_info_t *)Out0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ return check_result; }
#endif	/* defined(__MIG_check__Reply__host_virtual_physical_table_info_t__defined) */

	*info = (hash_info_bucket_array_t)(Out0P->info.address);
	*infoCnt = Out0P->infoCnt;

	return KERN_SUCCESS;
}

#if ( __MigTypeCheck || __NDR_convert__ )
#if __MIG_check__Reply__mach_host_subsystem__
#if !defined(__MIG_check__Reply__host_ipc_hash_info_t__defined)
#define __MIG_check__Reply__host_ipc_hash_info_t__defined
#ifndef __NDR_convert__int_rep__Reply__host_ipc_hash_info_t__info__defined
#if	defined(__NDR_convert__int_rep__mach_host__hash_info_bucket_array_t__defined)
#define	__NDR_convert__int_rep__Reply__host_ipc_hash_info_t__info__defined
#define	__NDR_convert__int_rep__Reply__host_ipc_hash_info_t__info(a, f, c) \
	__NDR_convert__int_rep__mach_host__hash_info_bucket_array_t((hash_info_bucket_array_t *)(a), f, c)
#elif	defined(__NDR_convert__int_rep__hash_info_bucket_array_t__defined)
#define	__NDR_convert__int_rep__Reply__host_ipc_hash_info_t__info__defined
#define	__NDR_convert__int_rep__Reply__host_ipc_hash_info_t__info(a, f, c) \
	__NDR_convert__int_rep__hash_info_bucket_array_t((hash_info_bucket_array_t *)(a), f, c)
#elif	defined(__NDR_convert__int_rep__mach_host__hash_info_bucket_t__defined)
#define	__NDR_convert__int_rep__Reply__host_ipc_hash_info_t__info__defined
#define	__NDR_convert__int_rep__Reply__host_ipc_hash_info_t__info(a, f, c) \
	__NDR_convert__ARRAY((hash_info_bucket_t *)(a), f, c, __NDR_convert__int_rep__mach_host__hash_info_bucket_t)
#elif	defined(__NDR_convert__int_rep__hash_info_bucket_t__defined)
#define	__NDR_convert__int_rep__Reply__host_ipc_hash_info_t__info__defined
#define	__NDR_convert__int_rep__Reply__host_ipc_hash_info_t__info(a, f, c) \
	__NDR_convert__ARRAY((hash_info_bucket_t *)(a), f, c, __NDR_convert__int_rep__hash_info_bucket_t)
#elif	defined(__NDR_convert__int_rep__mach_host__natural_t__defined)
#define	__NDR_convert__int_rep__Reply__host_ipc_hash_info_t__info__defined
#define	__NDR_convert__int_rep__Reply__host_ipc_hash_info_t__info(a, f, c) \
	__NDR_convert__ARRAY((natural_t *)(a), f, c, __NDR_convert__int_rep__mach_host__natural_t)
#elif	defined(__NDR_convert__int_rep__natural_t__defined)
#define	__NDR_convert__int_rep__Reply__host_ipc_hash_info_t__info__defined
#define	__NDR_convert__int_rep__Reply__host_ipc_hash_info_t__info(a, f, c) \
	__NDR_convert__ARRAY((natural_t *)(a), f, c, __NDR_convert__int_rep__natural_t)
#elif	defined(__NDR_convert__int_rep__mach_host__natural_t__defined)
#define	__NDR_convert__int_rep__Reply__host_ipc_hash_info_t__info__defined
#define	__NDR_convert__int_rep__Reply__host_ipc_hash_info_t__info(a, f, c) \
	__NDR_convert__ARRAY((natural_t *)(a), f, c, __NDR_convert__int_rep__mach_host__natural_t)
#elif	defined(__NDR_convert__int_rep__natural_t__defined)
#define	__NDR_convert__int_rep__Reply__host_ipc_hash_info_t__info__defined
#define	__NDR_convert__int_rep__Reply__host_ipc_hash_info_t__info(a, f, c) \
	__NDR_convert__ARRAY((natural_t *)(a), f, c, __NDR_convert__int_rep__natural_t)
#elif	defined(__NDR_convert__int_rep__mach_host__uint32_t__defined)
#define	__NDR_convert__int_rep__Reply__host_ipc_hash_info_t__info__defined
#define	__NDR_convert__int_rep__Reply__host_ipc_hash_info_t__info(a, f, c) \
	__NDR_convert__ARRAY((uint32_t *)(a), f, c, __NDR_convert__int_rep__mach_host__uint32_t)
#elif	defined(__NDR_convert__int_rep__uint32_t__defined)
#define	__NDR_convert__int_rep__Reply__host_ipc_hash_info_t__info__defined
#define	__NDR_convert__int_rep__Reply__host_ipc_hash_info_t__info(a, f, c) \
	__NDR_convert__ARRAY((uint32_t *)(a), f, c, __NDR_convert__int_rep__uint32_t)
#endif /* defined(__NDR_convert__*__defined) */
#endif /* __NDR_convert__int_rep__Reply__host_ipc_hash_info_t__info__defined */


#ifndef __NDR_convert__int_rep__Reply__host_ipc_hash_info_t__infoCnt__defined
#if	defined(__NDR_convert__int_rep__mach_host__mach_msg_type_number_t__defined)
#define	__NDR_convert__int_rep__Reply__host_ipc_hash_info_t__infoCnt__defined
#define	__NDR_convert__int_rep__Reply__host_ipc_hash_info_t__infoCnt(a, f) \
	__NDR_convert__int_rep__mach_host__mach_msg_type_number_t((mach_msg_type_number_t *)(a), f)
#elif	defined(__NDR_convert__int_rep__mach_msg_type_number_t__defined)
#define	__NDR_convert__int_rep__Reply__host_ipc_hash_info_t__infoCnt__defined
#define	__NDR_convert__int_rep__Reply__host_ipc_hash_info_t__infoCnt(a, f) \
	__NDR_convert__int_rep__mach_msg_type_number_t((mach_msg_type_number_t *)(a), f)
#endif /* defined(__NDR_convert__*__defined) */
#endif /* __NDR_convert__int_rep__Reply__host_ipc_hash_info_t__infoCnt__defined */


#ifndef __NDR_convert__char_rep__Reply__host_ipc_hash_info_t__info__defined
#if	defined(__NDR_convert__char_rep__mach_host__hash_info_bucket_array_t__defined)
#define	__NDR_convert__char_rep__Reply__host_ipc_hash_info_t__info__defined
#define	__NDR_convert__char_rep__Reply__host_ipc_hash_info_t__info(a, f, c) \
	__NDR_convert__char_rep__mach_host__hash_info_bucket_array_t((hash_info_bucket_array_t *)(a), f, c)
#elif	defined(__NDR_convert__char_rep__hash_info_bucket_array_t__defined)
#define	__NDR_convert__char_rep__Reply__host_ipc_hash_info_t__info__defined
#define	__NDR_convert__char_rep__Reply__host_ipc_hash_info_t__info(a, f, c) \
	__NDR_convert__char_rep__hash_info_bucket_array_t((hash_info_bucket_array_t *)(a), f, c)
#elif	defined(__NDR_convert__char_rep__mach_host__hash_info_bucket_t__defined)
#define	__NDR_convert__char_rep__Reply__host_ipc_hash_info_t__info__defined
#define	__NDR_convert__char_rep__Reply__host_ipc_hash_info_t__info(a, f, c) \
	__NDR_convert__ARRAY((hash_info_bucket_t *)(a), f, c, __NDR_convert__char_rep__mach_host__hash_info_bucket_t)
#elif	defined(__NDR_convert__char_rep__hash_info_bucket_t__defined)
#define	__NDR_convert__char_rep__Reply__host_ipc_hash_info_t__info__defined
#define	__NDR_convert__char_rep__Reply__host_ipc_hash_info_t__info(a, f, c) \
	__NDR_convert__ARRAY((hash_info_bucket_t *)(a), f, c, __NDR_convert__char_rep__hash_info_bucket_t)
#elif	defined(__NDR_convert__char_rep__mach_host__natural_t__defined)
#define	__NDR_convert__char_rep__Reply__host_ipc_hash_info_t__info__defined
#define	__NDR_convert__char_rep__Reply__host_ipc_hash_info_t__info(a, f, c) \
	__NDR_convert__ARRAY((natural_t *)(a), f, c, __NDR_convert__char_rep__mach_host__natural_t)
#elif	defined(__NDR_convert__char_rep__natural_t__defined)
#define	__NDR_convert__char_rep__Reply__host_ipc_hash_info_t__info__defined
#define	__NDR_convert__char_rep__Reply__host_ipc_hash_info_t__info(a, f, c) \
	__NDR_convert__ARRAY((natural_t *)(a), f, c, __NDR_convert__char_rep__natural_t)
#elif	defined(__NDR_convert__char_rep__mach_host__natural_t__defined)
#define	__NDR_convert__char_rep__Reply__host_ipc_hash_info_t__info__defined
#define	__NDR_convert__char_rep__Reply__host_ipc_hash_info_t__info(a, f, c) \
	__NDR_convert__ARRAY((natural_t *)(a), f, c, __NDR_convert__char_rep__mach_host__natural_t)
#elif	defined(__NDR_convert__char_rep__natural_t__defined)
#define	__NDR_convert__char_rep__Reply__host_ipc_hash_info_t__info__defined
#define	__NDR_convert__char_rep__Reply__host_ipc_hash_info_t__info(a, f, c) \
	__NDR_convert__ARRAY((natural_t *)(a), f, c, __NDR_convert__char_rep__natural_t)
#elif	defined(__NDR_convert__char_rep__mach_host__uint32_t__defined)
#define	__NDR_convert__char_rep__Reply__host_ipc_hash_info_t__info__defined
#define	__NDR_convert__char_rep__Reply__host_ipc_hash_info_t__info(a, f, c) \
	__NDR_convert__ARRAY((uint32_t *)(a), f, c, __NDR_convert__char_rep__mach_host__uint32_t)
#elif	defined(__NDR_convert__char_rep__uint32_t__defined)
#define	__NDR_convert__char_rep__Reply__host_ipc_hash_info_t__info__defined
#define	__NDR_convert__char_rep__Reply__host_ipc_hash_info_t__info(a, f, c) \
	__NDR_convert__ARRAY((uint32_t *)(a), f, c, __NDR_convert__char_rep__uint32_t)
#endif /* defined(__NDR_convert__*__defined) */
#endif /* __NDR_convert__char_rep__Reply__host_ipc_hash_info_t__info__defined */



#ifndef __NDR_convert__float_rep__Reply__host_ipc_hash_info_t__info__defined
#if	defined(__NDR_convert__float_rep__mach_host__hash_info_bucket_array_t__defined)
#define	__NDR_convert__float_rep__Reply__host_ipc_hash_info_t__info__defined
#define	__NDR_convert__float_rep__Reply__host_ipc_hash_info_t__info(a, f, c) \
	__NDR_convert__float_rep__mach_host__hash_info_bucket_array_t((hash_info_bucket_array_t *)(a), f, c)
#elif	defined(__NDR_convert__float_rep__hash_info_bucket_array_t__defined)
#define	__NDR_convert__float_rep__Reply__host_ipc_hash_info_t__info__defined
#define	__NDR_convert__float_rep__Reply__host_ipc_hash_info_t__info(a, f, c) \
	__NDR_convert__float_rep__hash_info_bucket_array_t((hash_info_bucket_array_t *)(a), f, c)
#elif	defined(__NDR_convert__float_rep__mach_host__hash_info_bucket_t__defined)
#define	__NDR_convert__float_rep__Reply__host_ipc_hash_info_t__info__defined
#define	__NDR_convert__float_rep__Reply__host_ipc_hash_info_t__info(a, f, c) \
	__NDR_convert__ARRAY((hash_info_bucket_t *)(a), f, c, __NDR_convert__float_rep__mach_host__hash_info_bucket_t)
#elif	defined(__NDR_convert__float_rep__hash_info_bucket_t__defined)
#define	__NDR_convert__float_rep__Reply__host_ipc_hash_info_t__info__defined
#define	__NDR_convert__float_rep__Reply__host_ipc_hash_info_t__info(a, f, c) \
	__NDR_convert__ARRAY((hash_info_bucket_t *)(a), f, c, __NDR_convert__float_rep__hash_info_bucket_t)
#elif	defined(__NDR_convert__float_rep__mach_host__natural_t__defined)
#define	__NDR_convert__float_rep__Reply__host_ipc_hash_info_t__info__defined
#define	__NDR_convert__float_rep__Reply__host_ipc_hash_info_t__info(a, f, c) \
	__NDR_convert__ARRAY((natural_t *)(a), f, c, __NDR_convert__float_rep__mach_host__natural_t)
#elif	defined(__NDR_convert__float_rep__natural_t__defined)
#define	__NDR_convert__float_rep__Reply__host_ipc_hash_info_t__info__defined
#define	__NDR_convert__float_rep__Reply__host_ipc_hash_info_t__info(a, f, c) \
	__NDR_convert__ARRAY((natural_t *)(a), f, c, __NDR_convert__float_rep__natural_t)
#elif	defined(__NDR_convert__float_rep__mach_host__natural_t__defined)
#define	__NDR_convert__float_rep__Reply__host_ipc_hash_info_t__info__defined
#define	__NDR_convert__float_rep__Reply__host_ipc_hash_info_t__info(a, f, c) \
	__NDR_convert__ARRAY((natural_t *)(a), f, c, __NDR_convert__float_rep__mach_host__natural_t)
#elif	defined(__NDR_convert__float_rep__natural_t__defined)
#define	__NDR_convert__float_rep__Reply__host_ipc_hash_info_t__info__defined
#define	__NDR_convert__float_rep__Reply__host_ipc_hash_info_t__info(a, f, c) \
	__NDR_convert__ARRAY((natural_t *)(a), f, c, __NDR_convert__float_rep__natural_t)
#elif	defined(__NDR_convert__float_rep__mach_host__uint32_t__defined)
#define	__NDR_convert__float_rep__Reply__host_ipc_hash_info_t__info__defined
#define	__NDR_convert__float_rep__Reply__host_ipc_hash_info_t__info(a, f, c) \
	__NDR_convert__ARRAY((uint32_t *)(a), f, c, __NDR_convert__float_rep__mach_host__uint32_t)
#elif	defined(__NDR_convert__float_rep__uint32_t__defined)
#define	__NDR_convert__float_rep__Reply__host_ipc_hash_info_t__info__defined
#define	__NDR_convert__float_rep__Reply__host_ipc_hash_info_t__info(a, f, c) \
	__NDR_convert__ARRAY((uint32_t *)(a), f, c, __NDR_convert__float_rep__uint32_t)
#endif /* defined(__NDR_convert__*__defined) */
#endif /* __NDR_convert__float_rep__Reply__host_ipc_hash_info_t__info__defined */




mig_internal kern_return_t __MIG_check__Reply__host_ipc_hash_info_t(__Reply__host_ipc_hash_info_t *Out0P)
{

	typedef __Reply__host_ipc_hash_info_t __Reply;
	boolean_t msgh_simple;
#if	__MigTypeCheck
	unsigned int msgh_size;
#endif	/* __MigTypeCheck */
	if (Out0P->Head.msgh_id != 310) {
	    if (Out0P->Head.msgh_id == MACH_NOTIFY_SEND_ONCE)
		{ return MIG_SERVER_DIED; }
	    else
		{ return MIG_REPLY_MISMATCH; }
	}

	msgh_simple = !(Out0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX);
#if	__MigTypeCheck
	msgh_size = Out0P->Head.msgh_size;

	if ((msgh_simple || Out0P->msgh_body.msgh_descriptor_count != 1 ||
	    msgh_size != (mach_msg_size_t)sizeof(__Reply)) &&
	    (!msgh_simple || msgh_size != (mach_msg_size_t)sizeof(mig_reply_error_t) ||
	    ((mig_reply_error_t *)Out0P)->RetCode == KERN_SUCCESS))
		{ return MIG_TYPE_ERROR ; }
#endif	/* __MigTypeCheck */

	if (msgh_simple) {
#ifdef	__NDR_convert__mig_reply_error_t__defined
		__NDR_convert__mig_reply_error_t((mig_reply_error_t *)Out0P);
#endif	/* __NDR_convert__mig_reply_error_t__defined */
		return ((mig_reply_error_t *)Out0P)->RetCode;
	}

#if	__MigTypeCheck
	if (Out0P->info.type != MACH_MSG_OOL_DESCRIPTOR) {
		return MIG_TYPE_ERROR;
	}
#endif	/* __MigTypeCheck */

#if	defined(__NDR_convert__int_rep__Reply__host_ipc_hash_info_t__info__defined) || \
	defined(__NDR_convert__int_rep__Reply__host_ipc_hash_info_t__infoCnt__defined)
	if (Out0P->NDR.int_rep != NDR_record.int_rep) {
#if defined(__NDR_convert__int_rep__Reply__host_ipc_hash_info_t__infoCnt__defined)
		__NDR_convert__int_rep__Reply__host_ipc_hash_info_t__infoCnt(&Out0P->infoCnt, Out0P->NDR.int_rep);
#endif	/* __NDR_convert__int_rep__Reply__host_ipc_hash_info_t__infoCnt__defined */
#if defined(__NDR_convert__int_rep__Reply__host_ipc_hash_info_t__info__defined)
		__NDR_convert__int_rep__Reply__host_ipc_hash_info_t__info((hash_info_bucket_array_t)(Out0P->info.address), Out0P->NDR.int_rep, Out0P->infoCnt);
#endif /* __NDR_convert__int_rep__Reply__host_ipc_hash_info_t__info__defined */
	}
#endif	/* defined(__NDR_convert__int_rep...) */

#if	defined(__NDR_convert__char_rep__Reply__host_ipc_hash_info_t__info__defined) || \
	0
	if (Out0P->NDR.char_rep != NDR_record.char_rep) {
#if defined(__NDR_convert__char_rep__Reply__host_ipc_hash_info_t__info__defined)
		__NDR_convert__char_rep__Reply__host_ipc_hash_info_t__info((hash_info_bucket_array_t)(Out0P->info.address), Out0P->NDR.char_rep, Out0P->infoCnt);
#endif /* __NDR_convert__char_rep__Reply__host_ipc_hash_info_t__info__defined */
	}
#endif	/* defined(__NDR_convert__char_rep...) */

#if	defined(__NDR_convert__float_rep__Reply__host_ipc_hash_info_t__info__defined) || \
	0
	if (Out0P->NDR.float_rep != NDR_record.float_rep) {
#if defined(__NDR_convert__float_rep__Reply__host_ipc_hash_info_t__info__defined)
		__NDR_convert__float_rep__Reply__host_ipc_hash_info_t__info((hash_info_bucket_array_t)(Out0P->info.address), Out0P->NDR.float_rep, Out0P->infoCnt);
#endif /* __NDR_convert__float_rep__Reply__host_ipc_hash_info_t__info__defined */
	}
#endif	/* defined(__NDR_convert__float_rep...) */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Reply__host_ipc_hash_info_t__defined) */
#endif /* __MIG_check__Reply__mach_host_subsystem__ */
#endif /* ( __MigTypeCheck || __NDR_convert__ ) */


/* Routine host_ipc_hash_info */
mig_external kern_return_t host_ipc_hash_info
(
	host_t host,
	hash_info_bucket_array_t *info,
	mach_msg_type_number_t *infoCnt
)
{

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
	} Request;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		mach_msg_ool_descriptor_t info;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		mach_msg_type_number_t infoCnt;
		mach_msg_trailer_t trailer;
	} Reply;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		mach_msg_ool_descriptor_t info;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		mach_msg_type_number_t infoCnt;
	} __Reply;
#ifdef  __MigPackStructs
#pragma pack()
#endif
	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	union {
		Request In;
		Reply Out;
	} Mess;

	Request *InP = &Mess.In;
	Reply *Out0P = &Mess.Out;

	mach_msg_return_t msg_result;

#ifdef	__MIG_check__Reply__host_ipc_hash_info_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Reply__host_ipc_hash_info_t__defined */

	__DeclareSendRpc(210, "host_ipc_hash_info")

	InP->Head.msgh_bits =
		MACH_MSGH_BITS(19, MACH_MSG_TYPE_MAKE_SEND_ONCE);
	/* msgh_size passed as argument */
	InP->Head.msgh_request_port = host;
	InP->Head.msgh_reply_port = mig_get_reply_port();
	InP->Head.msgh_id = 210;

	__BeforeSendRpc(210, "host_ipc_hash_info")
	msg_result = mach_msg(&InP->Head, MACH_SEND_MSG|MACH_RCV_MSG|MACH_MSG_OPTION_NONE, (mach_msg_size_t)sizeof(Request), (mach_msg_size_t)sizeof(Reply), InP->Head.msgh_reply_port, MACH_MSG_TIMEOUT_NONE, MACH_PORT_NULL);
	__AfterSendRpc(210, "host_ipc_hash_info")
	if (msg_result != MACH_MSG_SUCCESS) {
		__MachMsgErrorWithoutTimeout(msg_result);
		{ return msg_result; }
	}


#if	defined(__MIG_check__Reply__host_ipc_hash_info_t__defined)
	check_result = __MIG_check__Reply__host_ipc_hash_info_t((__Reply__host_ipc_hash_info_t *)Out0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ return check_result; }
#endif	/* defined(__MIG_check__Reply__host_ipc_hash_info_t__defined) */

	*info = (hash_info_bucket_array_t)(Out0P->info.address);
	*infoCnt = Out0P->infoCnt;

	return KERN_SUCCESS;
}

#if ( __MigTypeCheck || __NDR_convert__ )
#if __MIG_check__Reply__mach_host_subsystem__
#if !defined(__MIG_check__Reply__processor_set_default_t__defined)
#define __MIG_check__Reply__processor_set_default_t__defined

mig_internal kern_return_t __MIG_check__Reply__processor_set_default_t(__Reply__processor_set_default_t *Out0P)
{

	typedef __Reply__processor_set_default_t __Reply;
	boolean_t msgh_simple;
#if	__MigTypeCheck
	unsigned int msgh_size;
#endif	/* __MigTypeCheck */
	if (Out0P->Head.msgh_id != 313) {
	    if (Out0P->Head.msgh_id == MACH_NOTIFY_SEND_ONCE)
		{ return MIG_SERVER_DIED; }
	    else
		{ return MIG_REPLY_MISMATCH; }
	}

	msgh_simple = !(Out0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX);
#if	__MigTypeCheck
	msgh_size = Out0P->Head.msgh_size;

	if ((msgh_simple || Out0P->msgh_body.msgh_descriptor_count != 1 ||
	    msgh_size != (mach_msg_size_t)sizeof(__Reply)) &&
	    (!msgh_simple || msgh_size != (mach_msg_size_t)sizeof(mig_reply_error_t) ||
	    ((mig_reply_error_t *)Out0P)->RetCode == KERN_SUCCESS))
		{ return MIG_TYPE_ERROR ; }
#endif	/* __MigTypeCheck */

	if (msgh_simple) {
#ifdef	__NDR_convert__mig_reply_error_t__defined
		__NDR_convert__mig_reply_error_t((mig_reply_error_t *)Out0P);
#endif	/* __NDR_convert__mig_reply_error_t__defined */
		return ((mig_reply_error_t *)Out0P)->RetCode;
	}

#if	__MigTypeCheck
	if (Out0P->default_set.type != MACH_MSG_PORT_DESCRIPTOR ||
	    Out0P->default_set.disposition != 17) {
		return MIG_TYPE_ERROR;
	}
#endif	/* __MigTypeCheck */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Reply__processor_set_default_t__defined) */
#endif /* __MIG_check__Reply__mach_host_subsystem__ */
#endif /* ( __MigTypeCheck || __NDR_convert__ ) */


/* Routine processor_set_default */
mig_external kern_return_t processor_set_default
(
	host_t host,
	processor_set_name_t *default_set
)
{

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
	} Request;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		mach_msg_port_descriptor_t default_set;
		/* end of the kernel processed data */
		mach_msg_trailer_t trailer;
	} Reply;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		mach_msg_port_descriptor_t default_set;
		/* end of the kernel processed data */
	} __Reply;
#ifdef  __MigPackStructs
#pragma pack()
#endif
	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	union {
		Request In;
		Reply Out;
	} Mess;

	Request *InP = &Mess.In;
	Reply *Out0P = &Mess.Out;

	mach_msg_return_t msg_result;

#ifdef	__MIG_check__Reply__processor_set_default_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Reply__processor_set_default_t__defined */

	__DeclareSendRpc(213, "processor_set_default")

	InP->Head.msgh_bits =
		MACH_MSGH_BITS(19, MACH_MSG_TYPE_MAKE_SEND_ONCE);
	/* msgh_size passed as argument */
	InP->Head.msgh_request_port = host;
	InP->Head.msgh_reply_port = mig_get_reply_port();
	InP->Head.msgh_id = 213;

	__BeforeSendRpc(213, "processor_set_default")
	msg_result = mach_msg(&InP->Head, MACH_SEND_MSG|MACH_RCV_MSG|MACH_MSG_OPTION_NONE, (mach_msg_size_t)sizeof(Request), (mach_msg_size_t)sizeof(Reply), InP->Head.msgh_reply_port, MACH_MSG_TIMEOUT_NONE, MACH_PORT_NULL);
	__AfterSendRpc(213, "processor_set_default")
	if (msg_result != MACH_MSG_SUCCESS) {
		__MachMsgErrorWithoutTimeout(msg_result);
		{ return msg_result; }
	}


#if	defined(__MIG_check__Reply__processor_set_default_t__defined)
	check_result = __MIG_check__Reply__processor_set_default_t((__Reply__processor_set_default_t *)Out0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ return check_result; }
#endif	/* defined(__MIG_check__Reply__processor_set_default_t__defined) */

	*default_set = Out0P->default_set.name;
	return KERN_SUCCESS;
}

#if ( __MigTypeCheck || __NDR_convert__ )
#if __MIG_check__Reply__mach_host_subsystem__
#if !defined(__MIG_check__Reply__processor_set_create_t__defined)
#define __MIG_check__Reply__processor_set_create_t__defined

mig_internal kern_return_t __MIG_check__Reply__processor_set_create_t(__Reply__processor_set_create_t *Out0P)
{

	typedef __Reply__processor_set_create_t __Reply;
	boolean_t msgh_simple;
#if	__MigTypeCheck
	unsigned int msgh_size;
#endif	/* __MigTypeCheck */
	if (Out0P->Head.msgh_id != 314) {
	    if (Out0P->Head.msgh_id == MACH_NOTIFY_SEND_ONCE)
		{ return MIG_SERVER_DIED; }
	    else
		{ return MIG_REPLY_MISMATCH; }
	}

	msgh_simple = !(Out0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX);
#if	__MigTypeCheck
	msgh_size = Out0P->Head.msgh_size;

	if ((msgh_simple || Out0P->msgh_body.msgh_descriptor_count != 2 ||
	    msgh_size != (mach_msg_size_t)sizeof(__Reply)) &&
	    (!msgh_simple || msgh_size != (mach_msg_size_t)sizeof(mig_reply_error_t) ||
	    ((mig_reply_error_t *)Out0P)->RetCode == KERN_SUCCESS))
		{ return MIG_TYPE_ERROR ; }
#endif	/* __MigTypeCheck */

	if (msgh_simple) {
#ifdef	__NDR_convert__mig_reply_error_t__defined
		__NDR_convert__mig_reply_error_t((mig_reply_error_t *)Out0P);
#endif	/* __NDR_convert__mig_reply_error_t__defined */
		return ((mig_reply_error_t *)Out0P)->RetCode;
	}

#if	__MigTypeCheck
	if (Out0P->new_set.type != MACH_MSG_PORT_DESCRIPTOR ||
	    Out0P->new_set.disposition != 17) {
		return MIG_TYPE_ERROR;
	}
#endif	/* __MigTypeCheck */

#if	__MigTypeCheck
	if (Out0P->new_name.type != MACH_MSG_PORT_DESCRIPTOR ||
	    Out0P->new_name.disposition != 17) {
		return MIG_TYPE_ERROR;
	}
#endif	/* __MigTypeCheck */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Reply__processor_set_create_t__defined) */
#endif /* __MIG_check__Reply__mach_host_subsystem__ */
#endif /* ( __MigTypeCheck || __NDR_convert__ ) */


/* Routine processor_set_create */
mig_external kern_return_t processor_set_create
(
	host_t host,
	processor_set_t *new_set,
	processor_set_name_t *new_name
)
{

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
	} Request;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		mach_msg_port_descriptor_t new_set;
		mach_msg_port_descriptor_t new_name;
		/* end of the kernel processed data */
		mach_msg_trailer_t trailer;
	} Reply;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		mach_msg_port_descriptor_t new_set;
		mach_msg_port_descriptor_t new_name;
		/* end of the kernel processed data */
	} __Reply;
#ifdef  __MigPackStructs
#pragma pack()
#endif
	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	union {
		Request In;
		Reply Out;
	} Mess;

	Request *InP = &Mess.In;
	Reply *Out0P = &Mess.Out;

	mach_msg_return_t msg_result;

#ifdef	__MIG_check__Reply__processor_set_create_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Reply__processor_set_create_t__defined */

	__DeclareSendRpc(214, "processor_set_create")

	InP->Head.msgh_bits =
		MACH_MSGH_BITS(19, MACH_MSG_TYPE_MAKE_SEND_ONCE);
	/* msgh_size passed as argument */
	InP->Head.msgh_request_port = host;
	InP->Head.msgh_reply_port = mig_get_reply_port();
	InP->Head.msgh_id = 214;

	__BeforeSendRpc(214, "processor_set_create")
	msg_result = mach_msg(&InP->Head, MACH_SEND_MSG|MACH_RCV_MSG|MACH_MSG_OPTION_NONE, (mach_msg_size_t)sizeof(Request), (mach_msg_size_t)sizeof(Reply), InP->Head.msgh_reply_port, MACH_MSG_TIMEOUT_NONE, MACH_PORT_NULL);
	__AfterSendRpc(214, "processor_set_create")
	if (msg_result != MACH_MSG_SUCCESS) {
		__MachMsgErrorWithoutTimeout(msg_result);
		{ return msg_result; }
	}


#if	defined(__MIG_check__Reply__processor_set_create_t__defined)
	check_result = __MIG_check__Reply__processor_set_create_t((__Reply__processor_set_create_t *)Out0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ return check_result; }
#endif	/* defined(__MIG_check__Reply__processor_set_create_t__defined) */

	*new_set = Out0P->new_set.name;
	*new_name = Out0P->new_name.name;
	return KERN_SUCCESS;
}

#if ( __MigTypeCheck || __NDR_convert__ )
#if __MIG_check__Reply__mach_host_subsystem__
#if !defined(__MIG_check__Reply__mach_memory_object_memory_entry_64_t__defined)
#define __MIG_check__Reply__mach_memory_object_memory_entry_64_t__defined

mig_internal kern_return_t __MIG_check__Reply__mach_memory_object_memory_entry_64_t(__Reply__mach_memory_object_memory_entry_64_t *Out0P)
{

	typedef __Reply__mach_memory_object_memory_entry_64_t __Reply;
	boolean_t msgh_simple;
#if	__MigTypeCheck
	unsigned int msgh_size;
#endif	/* __MigTypeCheck */
	if (Out0P->Head.msgh_id != 315) {
	    if (Out0P->Head.msgh_id == MACH_NOTIFY_SEND_ONCE)
		{ return MIG_SERVER_DIED; }
	    else
		{ return MIG_REPLY_MISMATCH; }
	}

	msgh_simple = !(Out0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX);
#if	__MigTypeCheck
	msgh_size = Out0P->Head.msgh_size;

	if ((msgh_simple || Out0P->msgh_body.msgh_descriptor_count != 1 ||
	    msgh_size != (mach_msg_size_t)sizeof(__Reply)) &&
	    (!msgh_simple || msgh_size != (mach_msg_size_t)sizeof(mig_reply_error_t) ||
	    ((mig_reply_error_t *)Out0P)->RetCode == KERN_SUCCESS))
		{ return MIG_TYPE_ERROR ; }
#endif	/* __MigTypeCheck */

	if (msgh_simple) {
#ifdef	__NDR_convert__mig_reply_error_t__defined
		__NDR_convert__mig_reply_error_t((mig_reply_error_t *)Out0P);
#endif	/* __NDR_convert__mig_reply_error_t__defined */
		return ((mig_reply_error_t *)Out0P)->RetCode;
	}

#if	__MigTypeCheck
	if (Out0P->entry_handle.type != MACH_MSG_PORT_DESCRIPTOR ||
	    Out0P->entry_handle.disposition != 17) {
		return MIG_TYPE_ERROR;
	}
#endif	/* __MigTypeCheck */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Reply__mach_memory_object_memory_entry_64_t__defined) */
#endif /* __MIG_check__Reply__mach_host_subsystem__ */
#endif /* ( __MigTypeCheck || __NDR_convert__ ) */


/* Routine mach_memory_object_memory_entry_64 */
mig_external kern_return_t mach_memory_object_memory_entry_64
(
	host_t host,
	boolean_t internal,
	memory_object_size_t size,
	vm_prot_t permission,
	memory_object_t pager,
	mach_port_t *entry_handle
)
{

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		mach_msg_port_descriptor_t pager;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		boolean_t internal;
		memory_object_size_t size;
		vm_prot_t permission;
	} Request;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		mach_msg_port_descriptor_t entry_handle;
		/* end of the kernel processed data */
		mach_msg_trailer_t trailer;
	} Reply;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		mach_msg_port_descriptor_t entry_handle;
		/* end of the kernel processed data */
	} __Reply;
#ifdef  __MigPackStructs
#pragma pack()
#endif
	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	union {
		Request In;
		Reply Out;
	} Mess;

	Request *InP = &Mess.In;
	Reply *Out0P = &Mess.Out;

	mach_msg_return_t msg_result;

#ifdef	__MIG_check__Reply__mach_memory_object_memory_entry_64_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Reply__mach_memory_object_memory_entry_64_t__defined */

	__DeclareSendRpc(215, "mach_memory_object_memory_entry_64")

#if	UseStaticTemplates
	const static mach_msg_port_descriptor_t pagerTemplate = {
		/* name = */		MACH_PORT_NULL,
		/* pad1 = */		0,
		/* pad2 = */		0,
		/* disp = */		19,
		/* type = */		MACH_MSG_PORT_DESCRIPTOR,
	};
#endif	/* UseStaticTemplates */

	InP->msgh_body.msgh_descriptor_count = 1;
#if	UseStaticTemplates
	InP->pager = pagerTemplate;
	InP->pager.name = pager;
#else	/* UseStaticTemplates */
	InP->pager.name = pager;
	InP->pager.disposition = 19;
	InP->pager.type = MACH_MSG_PORT_DESCRIPTOR;
#endif	/* UseStaticTemplates */

	InP->NDR = NDR_record;

	InP->internal = internal;

	InP->size = size;

	InP->permission = permission;

	InP->Head.msgh_bits = MACH_MSGH_BITS_COMPLEX|
		MACH_MSGH_BITS(19, MACH_MSG_TYPE_MAKE_SEND_ONCE);
	/* msgh_size passed as argument */
	InP->Head.msgh_request_port = host;
	InP->Head.msgh_reply_port = mig_get_reply_port();
	InP->Head.msgh_id = 215;

	__BeforeSendRpc(215, "mach_memory_object_memory_entry_64")
	msg_result = mach_msg(&InP->Head, MACH_SEND_MSG|MACH_RCV_MSG|MACH_MSG_OPTION_NONE, (mach_msg_size_t)sizeof(Request), (mach_msg_size_t)sizeof(Reply), InP->Head.msgh_reply_port, MACH_MSG_TIMEOUT_NONE, MACH_PORT_NULL);
	__AfterSendRpc(215, "mach_memory_object_memory_entry_64")
	if (msg_result != MACH_MSG_SUCCESS) {
		__MachMsgErrorWithoutTimeout(msg_result);
		{ return msg_result; }
	}


#if	defined(__MIG_check__Reply__mach_memory_object_memory_entry_64_t__defined)
	check_result = __MIG_check__Reply__mach_memory_object_memory_entry_64_t((__Reply__mach_memory_object_memory_entry_64_t *)Out0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ return check_result; }
#endif	/* defined(__MIG_check__Reply__mach_memory_object_memory_entry_64_t__defined) */

	*entry_handle = Out0P->entry_handle.name;
	return KERN_SUCCESS;
}

#if ( __MigTypeCheck || __NDR_convert__ )
#if __MIG_check__Reply__mach_host_subsystem__
#if !defined(__MIG_check__Reply__host_statistics_t__defined)
#define __MIG_check__Reply__host_statistics_t__defined
#ifndef __NDR_convert__int_rep__Reply__host_statistics_t__RetCode__defined
#if	defined(__NDR_convert__int_rep__mach_host__kern_return_t__defined)
#define	__NDR_convert__int_rep__Reply__host_statistics_t__RetCode__defined
#define	__NDR_convert__int_rep__Reply__host_statistics_t__RetCode(a, f) \
	__NDR_convert__int_rep__mach_host__kern_return_t((kern_return_t *)(a), f)
#elif	defined(__NDR_convert__int_rep__kern_return_t__defined)
#define	__NDR_convert__int_rep__Reply__host_statistics_t__RetCode__defined
#define	__NDR_convert__int_rep__Reply__host_statistics_t__RetCode(a, f) \
	__NDR_convert__int_rep__kern_return_t((kern_return_t *)(a), f)
#endif /* defined(__NDR_convert__*__defined) */
#endif /* __NDR_convert__int_rep__Reply__host_statistics_t__RetCode__defined */


#ifndef __NDR_convert__int_rep__Reply__host_statistics_t__host_info_out__defined
#if	defined(__NDR_convert__int_rep__mach_host__host_info_t__defined)
#define	__NDR_convert__int_rep__Reply__host_statistics_t__host_info_out__defined
#define	__NDR_convert__int_rep__Reply__host_statistics_t__host_info_out(a, f, c) \
	__NDR_convert__int_rep__mach_host__host_info_t((host_info_t *)(a), f, c)
#elif	defined(__NDR_convert__int_rep__host_info_t__defined)
#define	__NDR_convert__int_rep__Reply__host_statistics_t__host_info_out__defined
#define	__NDR_convert__int_rep__Reply__host_statistics_t__host_info_out(a, f, c) \
	__NDR_convert__int_rep__host_info_t((host_info_t *)(a), f, c)
#elif	defined(__NDR_convert__int_rep__mach_host__integer_t__defined)
#define	__NDR_convert__int_rep__Reply__host_statistics_t__host_info_out__defined
#define	__NDR_convert__int_rep__Reply__host_statistics_t__host_info_out(a, f, c) \
	__NDR_convert__ARRAY((integer_t *)(a), f, c, __NDR_convert__int_rep__mach_host__integer_t)
#elif	defined(__NDR_convert__int_rep__integer_t__defined)
#define	__NDR_convert__int_rep__Reply__host_statistics_t__host_info_out__defined
#define	__NDR_convert__int_rep__Reply__host_statistics_t__host_info_out(a, f, c) \
	__NDR_convert__ARRAY((integer_t *)(a), f, c, __NDR_convert__int_rep__integer_t)
#elif	defined(__NDR_convert__int_rep__mach_host__int32_t__defined)
#define	__NDR_convert__int_rep__Reply__host_statistics_t__host_info_out__defined
#define	__NDR_convert__int_rep__Reply__host_statistics_t__host_info_out(a, f, c) \
	__NDR_convert__ARRAY((int32_t *)(a), f, c, __NDR_convert__int_rep__mach_host__int32_t)
#elif	defined(__NDR_convert__int_rep__int32_t__defined)
#define	__NDR_convert__int_rep__Reply__host_statistics_t__host_info_out__defined
#define	__NDR_convert__int_rep__Reply__host_statistics_t__host_info_out(a, f, c) \
	__NDR_convert__ARRAY((int32_t *)(a), f, c, __NDR_convert__int_rep__int32_t)
#endif /* defined(__NDR_convert__*__defined) */
#endif /* __NDR_convert__int_rep__Reply__host_statistics_t__host_info_out__defined */


#ifndef __NDR_convert__int_rep__Reply__host_statistics_t__host_info_outCnt__defined
#if	defined(__NDR_convert__int_rep__mach_host__mach_msg_type_number_t__defined)
#define	__NDR_convert__int_rep__Reply__host_statistics_t__host_info_outCnt__defined
#define	__NDR_convert__int_rep__Reply__host_statistics_t__host_info_outCnt(a, f) \
	__NDR_convert__int_rep__mach_host__mach_msg_type_number_t((mach_msg_type_number_t *)(a), f)
#elif	defined(__NDR_convert__int_rep__mach_msg_type_number_t__defined)
#define	__NDR_convert__int_rep__Reply__host_statistics_t__host_info_outCnt__defined
#define	__NDR_convert__int_rep__Reply__host_statistics_t__host_info_outCnt(a, f) \
	__NDR_convert__int_rep__mach_msg_type_number_t((mach_msg_type_number_t *)(a), f)
#endif /* defined(__NDR_convert__*__defined) */
#endif /* __NDR_convert__int_rep__Reply__host_statistics_t__host_info_outCnt__defined */



#ifndef __NDR_convert__char_rep__Reply__host_statistics_t__host_info_out__defined
#if	defined(__NDR_convert__char_rep__mach_host__host_info_t__defined)
#define	__NDR_convert__char_rep__Reply__host_statistics_t__host_info_out__defined
#define	__NDR_convert__char_rep__Reply__host_statistics_t__host_info_out(a, f, c) \
	__NDR_convert__char_rep__mach_host__host_info_t((host_info_t *)(a), f, c)
#elif	defined(__NDR_convert__char_rep__host_info_t__defined)
#define	__NDR_convert__char_rep__Reply__host_statistics_t__host_info_out__defined
#define	__NDR_convert__char_rep__Reply__host_statistics_t__host_info_out(a, f, c) \
	__NDR_convert__char_rep__host_info_t((host_info_t *)(a), f, c)
#elif	defined(__NDR_convert__char_rep__mach_host__integer_t__defined)
#define	__NDR_convert__char_rep__Reply__host_statistics_t__host_info_out__defined
#define	__NDR_convert__char_rep__Reply__host_statistics_t__host_info_out(a, f, c) \
	__NDR_convert__ARRAY((integer_t *)(a), f, c, __NDR_convert__char_rep__mach_host__integer_t)
#elif	defined(__NDR_convert__char_rep__integer_t__defined)
#define	__NDR_convert__char_rep__Reply__host_statistics_t__host_info_out__defined
#define	__NDR_convert__char_rep__Reply__host_statistics_t__host_info_out(a, f, c) \
	__NDR_convert__ARRAY((integer_t *)(a), f, c, __NDR_convert__char_rep__integer_t)
#elif	defined(__NDR_convert__char_rep__mach_host__int32_t__defined)
#define	__NDR_convert__char_rep__Reply__host_statistics_t__host_info_out__defined
#define	__NDR_convert__char_rep__Reply__host_statistics_t__host_info_out(a, f, c) \
	__NDR_convert__ARRAY((int32_t *)(a), f, c, __NDR_convert__char_rep__mach_host__int32_t)
#elif	defined(__NDR_convert__char_rep__int32_t__defined)
#define	__NDR_convert__char_rep__Reply__host_statistics_t__host_info_out__defined
#define	__NDR_convert__char_rep__Reply__host_statistics_t__host_info_out(a, f, c) \
	__NDR_convert__ARRAY((int32_t *)(a), f, c, __NDR_convert__char_rep__int32_t)
#endif /* defined(__NDR_convert__*__defined) */
#endif /* __NDR_convert__char_rep__Reply__host_statistics_t__host_info_out__defined */




#ifndef __NDR_convert__float_rep__Reply__host_statistics_t__host_info_out__defined
#if	defined(__NDR_convert__float_rep__mach_host__host_info_t__defined)
#define	__NDR_convert__float_rep__Reply__host_statistics_t__host_info_out__defined
#define	__NDR_convert__float_rep__Reply__host_statistics_t__host_info_out(a, f, c) \
	__NDR_convert__float_rep__mach_host__host_info_t((host_info_t *)(a), f, c)
#elif	defined(__NDR_convert__float_rep__host_info_t__defined)
#define	__NDR_convert__float_rep__Reply__host_statistics_t__host_info_out__defined
#define	__NDR_convert__float_rep__Reply__host_statistics_t__host_info_out(a, f, c) \
	__NDR_convert__float_rep__host_info_t((host_info_t *)(a), f, c)
#elif	defined(__NDR_convert__float_rep__mach_host__integer_t__defined)
#define	__NDR_convert__float_rep__Reply__host_statistics_t__host_info_out__defined
#define	__NDR_convert__float_rep__Reply__host_statistics_t__host_info_out(a, f, c) \
	__NDR_convert__ARRAY((integer_t *)(a), f, c, __NDR_convert__float_rep__mach_host__integer_t)
#elif	defined(__NDR_convert__float_rep__integer_t__defined)
#define	__NDR_convert__float_rep__Reply__host_statistics_t__host_info_out__defined
#define	__NDR_convert__float_rep__Reply__host_statistics_t__host_info_out(a, f, c) \
	__NDR_convert__ARRAY((integer_t *)(a), f, c, __NDR_convert__float_rep__integer_t)
#elif	defined(__NDR_convert__float_rep__mach_host__int32_t__defined)
#define	__NDR_convert__float_rep__Reply__host_statistics_t__host_info_out__defined
#define	__NDR_convert__float_rep__Reply__host_statistics_t__host_info_out(a, f, c) \
	__NDR_convert__ARRAY((int32_t *)(a), f, c, __NDR_convert__float_rep__mach_host__int32_t)
#elif	defined(__NDR_convert__float_rep__int32_t__defined)
#define	__NDR_convert__float_rep__Reply__host_statistics_t__host_info_out__defined
#define	__NDR_convert__float_rep__Reply__host_statistics_t__host_info_out(a, f, c) \
	__NDR_convert__ARRAY((int32_t *)(a), f, c, __NDR_convert__float_rep__int32_t)
#endif /* defined(__NDR_convert__*__defined) */
#endif /* __NDR_convert__float_rep__Reply__host_statistics_t__host_info_out__defined */




mig_internal kern_return_t __MIG_check__Reply__host_statistics_t(__Reply__host_statistics_t *Out0P)
{

	typedef __Reply__host_statistics_t __Reply;
#if	__MigTypeCheck
	unsigned int msgh_size;
#endif	/* __MigTypeCheck */

	if (Out0P->Head.msgh_id != 316) {
	    if (Out0P->Head.msgh_id == MACH_NOTIFY_SEND_ONCE)
		{ return MIG_SERVER_DIED; }
	    else
		{ return MIG_REPLY_MISMATCH; }
	}

#if	__MigTypeCheck
	msgh_size = Out0P->Head.msgh_size;

	if ((Out0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) ||
	    ((msgh_size > (mach_msg_size_t)sizeof(__Reply) || msgh_size < (mach_msg_size_t)(sizeof(__Reply) - 60)) &&
	     (msgh_size != (mach_msg_size_t)sizeof(mig_reply_error_t) ||
	      Out0P->RetCode == KERN_SUCCESS)))
		{ return MIG_TYPE_ERROR ; }
#endif	/* __MigTypeCheck */

	if (Out0P->RetCode != KERN_SUCCESS) {
#ifdef	__NDR_convert__mig_reply_error_t__defined
		__NDR_convert__mig_reply_error_t((mig_reply_error_t *)Out0P);
#endif	/* __NDR_convert__mig_reply_error_t__defined */
		return ((mig_reply_error_t *)Out0P)->RetCode;
	}

#if defined(__NDR_convert__int_rep__Reply__host_statistics_t__host_info_outCnt__defined)
	if (Out0P->NDR.int_rep != NDR_record.int_rep)
		__NDR_convert__int_rep__Reply__host_statistics_t__host_info_outCnt(&Out0P->host_info_outCnt, Out0P->NDR.int_rep);
#endif	/* __NDR_convert__int_rep__Reply__host_statistics_t__host_info_outCnt__defined */
#if	__MigTypeCheck
	if (msgh_size != (mach_msg_size_t)(sizeof(__Reply) - 60) + ((4 * Out0P->host_info_outCnt)))
		{ return MIG_TYPE_ERROR ; }
#endif	/* __MigTypeCheck */

#if	defined(__NDR_convert__int_rep__Reply__host_statistics_t__RetCode__defined) || \
	defined(__NDR_convert__int_rep__Reply__host_statistics_t__host_info_out__defined) || \
	defined(__NDR_convert__int_rep__Reply__host_statistics_t__host_info_outCnt__defined)
	if (Out0P->NDR.int_rep != NDR_record.int_rep) {
#if defined(__NDR_convert__int_rep__Reply__host_statistics_t__RetCode__defined)
		__NDR_convert__int_rep__Reply__host_statistics_t__RetCode(&Out0P->RetCode, Out0P->NDR.int_rep);
#endif /* __NDR_convert__int_rep__Reply__host_statistics_t__RetCode__defined */
#if defined(__NDR_convert__int_rep__Reply__host_statistics_t__host_info_out__defined)
		__NDR_convert__int_rep__Reply__host_statistics_t__host_info_out(&Out0P->host_info_out, Out0P->NDR.int_rep, Out0P->host_info_outCnt);
#endif /* __NDR_convert__int_rep__Reply__host_statistics_t__host_info_out__defined */
	}
#endif	/* defined(__NDR_convert__int_rep...) */

#if	0 || \
	defined(__NDR_convert__char_rep__Reply__host_statistics_t__host_info_out__defined) || \
	0
	if (Out0P->NDR.char_rep != NDR_record.char_rep) {
#if defined(__NDR_convert__char_rep__Reply__host_statistics_t__host_info_out__defined)
		__NDR_convert__char_rep__Reply__host_statistics_t__host_info_out(&Out0P->host_info_out, Out0P->NDR.char_rep, Out0P->host_info_outCnt);
#endif /* __NDR_convert__char_rep__Reply__host_statistics_t__host_info_out__defined */
	}
#endif	/* defined(__NDR_convert__char_rep...) */

#if	0 || \
	defined(__NDR_convert__float_rep__Reply__host_statistics_t__host_info_out__defined) || \
	0
	if (Out0P->NDR.float_rep != NDR_record.float_rep) {
#if defined(__NDR_convert__float_rep__Reply__host_statistics_t__host_info_out__defined)
		__NDR_convert__float_rep__Reply__host_statistics_t__host_info_out(&Out0P->host_info_out, Out0P->NDR.float_rep, Out0P->host_info_outCnt);
#endif /* __NDR_convert__float_rep__Reply__host_statistics_t__host_info_out__defined */
	}
#endif	/* defined(__NDR_convert__float_rep...) */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Reply__host_statistics_t__defined) */
#endif /* __MIG_check__Reply__mach_host_subsystem__ */
#endif /* ( __MigTypeCheck || __NDR_convert__ ) */


/* Routine host_statistics */
mig_external kern_return_t host_statistics
(
	host_t host_priv,
	host_flavor_t flavor,
	host_info_t host_info_out,
	mach_msg_type_number_t *host_info_outCnt
)
{

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		NDR_record_t NDR;
		host_flavor_t flavor;
		mach_msg_type_number_t host_info_outCnt;
	} Request;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		NDR_record_t NDR;
		kern_return_t RetCode;
		mach_msg_type_number_t host_info_outCnt;
		integer_t host_info_out[15];
		mach_msg_trailer_t trailer;
	} Reply;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		NDR_record_t NDR;
		kern_return_t RetCode;
		mach_msg_type_number_t host_info_outCnt;
		integer_t host_info_out[15];
	} __Reply;
#ifdef  __MigPackStructs
#pragma pack()
#endif
	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	union {
		Request In;
		Reply Out;
	} Mess;

	Request *InP = &Mess.In;
	Reply *Out0P = &Mess.Out;

	mach_msg_return_t msg_result;

#ifdef	__MIG_check__Reply__host_statistics_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Reply__host_statistics_t__defined */

	__DeclareSendRpc(216, "host_statistics")

	InP->NDR = NDR_record;

	InP->flavor = flavor;

	if (*host_info_outCnt < 15)
		InP->host_info_outCnt = *host_info_outCnt;
	else
		InP->host_info_outCnt = 15;

	InP->Head.msgh_bits =
		MACH_MSGH_BITS(19, MACH_MSG_TYPE_MAKE_SEND_ONCE);
	/* msgh_size passed as argument */
	InP->Head.msgh_request_port = host_priv;
	InP->Head.msgh_reply_port = mig_get_reply_port();
	InP->Head.msgh_id = 216;

	__BeforeSendRpc(216, "host_statistics")
	msg_result = mach_msg(&InP->Head, MACH_SEND_MSG|MACH_RCV_MSG|MACH_MSG_OPTION_NONE, (mach_msg_size_t)sizeof(Request), (mach_msg_size_t)sizeof(Reply), InP->Head.msgh_reply_port, MACH_MSG_TIMEOUT_NONE, MACH_PORT_NULL);
	__AfterSendRpc(216, "host_statistics")
	if (msg_result != MACH_MSG_SUCCESS) {
		__MachMsgErrorWithoutTimeout(msg_result);
		{ return msg_result; }
	}


#if	defined(__MIG_check__Reply__host_statistics_t__defined)
	check_result = __MIG_check__Reply__host_statistics_t((__Reply__host_statistics_t *)Out0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ return check_result; }
#endif	/* defined(__MIG_check__Reply__host_statistics_t__defined) */

	if (Out0P->host_info_outCnt > *host_info_outCnt) {
		(void)memcpy((char *) host_info_out, (const char *) Out0P->host_info_out, 4 *  *host_info_outCnt);
		*host_info_outCnt = Out0P->host_info_outCnt;
		{ return MIG_ARRAY_TOO_LARGE; }
	}
	(void)memcpy((char *) host_info_out, (const char *) Out0P->host_info_out, 4 * Out0P->host_info_outCnt);

	*host_info_outCnt = Out0P->host_info_outCnt;

	return KERN_SUCCESS;
}

#if ( __MigTypeCheck || __NDR_convert__ )
#if __MIG_check__Reply__mach_host_subsystem__
#if !defined(__MIG_check__Reply__host_request_notification_t__defined)
#define __MIG_check__Reply__host_request_notification_t__defined
#ifndef __NDR_convert__int_rep__Reply__host_request_notification_t__RetCode__defined
#if	defined(__NDR_convert__int_rep__mach_host__kern_return_t__defined)
#define	__NDR_convert__int_rep__Reply__host_request_notification_t__RetCode__defined
#define	__NDR_convert__int_rep__Reply__host_request_notification_t__RetCode(a, f) \
	__NDR_convert__int_rep__mach_host__kern_return_t((kern_return_t *)(a), f)
#elif	defined(__NDR_convert__int_rep__kern_return_t__defined)
#define	__NDR_convert__int_rep__Reply__host_request_notification_t__RetCode__defined
#define	__NDR_convert__int_rep__Reply__host_request_notification_t__RetCode(a, f) \
	__NDR_convert__int_rep__kern_return_t((kern_return_t *)(a), f)
#endif /* defined(__NDR_convert__*__defined) */
#endif /* __NDR_convert__int_rep__Reply__host_request_notification_t__RetCode__defined */





mig_internal kern_return_t __MIG_check__Reply__host_request_notification_t(__Reply__host_request_notification_t *Out0P)
{

	typedef __Reply__host_request_notification_t __Reply;
	if (Out0P->Head.msgh_id != 317) {
	    if (Out0P->Head.msgh_id == MACH_NOTIFY_SEND_ONCE)
		{ return MIG_SERVER_DIED; }
	    else
		{ return MIG_REPLY_MISMATCH; }
	}

#if	__MigTypeCheck
	if ((Out0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) ||
	    (Out0P->Head.msgh_size != (mach_msg_size_t)sizeof(__Reply)))
		{ return MIG_TYPE_ERROR ; }
#endif	/* __MigTypeCheck */

#if defined(__NDR_convert__int_rep__Reply__host_request_notification_t__RetCode__defined)
	if (Out0P->NDR.int_rep != NDR_record.int_rep)
		__NDR_convert__int_rep__Reply__host_request_notification_t__RetCode(&Out0P->RetCode, Out0P->NDR.int_rep);
#endif	/* __NDR_convert__int_rep__Reply__host_request_notification_t__RetCode__defined */
	{
		return Out0P->RetCode;
	}
}
#endif /* !defined(__MIG_check__Reply__host_request_notification_t__defined) */
#endif /* __MIG_check__Reply__mach_host_subsystem__ */
#endif /* ( __MigTypeCheck || __NDR_convert__ ) */


/* Routine host_request_notification */
mig_external kern_return_t host_request_notification
(
	host_t host,
	host_flavor_t notify_type,
	mach_port_t notify_port
)
{

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		mach_msg_port_descriptor_t notify_port;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		host_flavor_t notify_type;
	} Request;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		NDR_record_t NDR;
		kern_return_t RetCode;
		mach_msg_trailer_t trailer;
	} Reply;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		NDR_record_t NDR;
		kern_return_t RetCode;
	} __Reply;
#ifdef  __MigPackStructs
#pragma pack()
#endif
	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	union {
		Request In;
		Reply Out;
	} Mess;

	Request *InP = &Mess.In;
	Reply *Out0P = &Mess.Out;

	mach_msg_return_t msg_result;

#ifdef	__MIG_check__Reply__host_request_notification_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Reply__host_request_notification_t__defined */

	__DeclareSendRpc(217, "host_request_notification")

#if	UseStaticTemplates
	const static mach_msg_port_descriptor_t notify_portTemplate = {
		/* name = */		MACH_PORT_NULL,
		/* pad1 = */		0,
		/* pad2 = */		0,
		/* disp = */		21,
		/* type = */		MACH_MSG_PORT_DESCRIPTOR,
	};
#endif	/* UseStaticTemplates */

	InP->msgh_body.msgh_descriptor_count = 1;
#if	UseStaticTemplates
	InP->notify_port = notify_portTemplate;
	InP->notify_port.name = notify_port;
#else	/* UseStaticTemplates */
	InP->notify_port.name = notify_port;
	InP->notify_port.disposition = 21;
	InP->notify_port.type = MACH_MSG_PORT_DESCRIPTOR;
#endif	/* UseStaticTemplates */

	InP->NDR = NDR_record;

	InP->notify_type = notify_type;

	InP->Head.msgh_bits = MACH_MSGH_BITS_COMPLEX|
		MACH_MSGH_BITS(19, MACH_MSG_TYPE_MAKE_SEND_ONCE);
	/* msgh_size passed as argument */
	InP->Head.msgh_request_port = host;
	InP->Head.msgh_reply_port = mig_get_reply_port();
	InP->Head.msgh_id = 217;

	__BeforeSendRpc(217, "host_request_notification")
	msg_result = mach_msg(&InP->Head, MACH_SEND_MSG|MACH_RCV_MSG|MACH_MSG_OPTION_NONE, (mach_msg_size_t)sizeof(Request), (mach_msg_size_t)sizeof(Reply), InP->Head.msgh_reply_port, MACH_MSG_TIMEOUT_NONE, MACH_PORT_NULL);
	__AfterSendRpc(217, "host_request_notification")
	if (msg_result != MACH_MSG_SUCCESS) {
		__MachMsgErrorWithoutTimeout(msg_result);
		{ return msg_result; }
	}


#if	defined(__MIG_check__Reply__host_request_notification_t__defined)
	check_result = __MIG_check__Reply__host_request_notification_t((__Reply__host_request_notification_t *)Out0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ return check_result; }
#endif	/* defined(__MIG_check__Reply__host_request_notification_t__defined) */

	return KERN_SUCCESS;
}

#if ( __MigTypeCheck || __NDR_convert__ )
#if __MIG_check__Reply__mach_host_subsystem__
#if !defined(__MIG_check__Reply__host_lockgroup_info_t__defined)
#define __MIG_check__Reply__host_lockgroup_info_t__defined
#ifndef __NDR_convert__int_rep__Reply__host_lockgroup_info_t__lockgroup_info__defined
#if	defined(__NDR_convert__int_rep__mach_host__lockgroup_info_array_t__defined)
#define	__NDR_convert__int_rep__Reply__host_lockgroup_info_t__lockgroup_info__defined
#define	__NDR_convert__int_rep__Reply__host_lockgroup_info_t__lockgroup_info(a, f, c) \
	__NDR_convert__int_rep__mach_host__lockgroup_info_array_t((lockgroup_info_array_t *)(a), f, c)
#elif	defined(__NDR_convert__int_rep__lockgroup_info_array_t__defined)
#define	__NDR_convert__int_rep__Reply__host_lockgroup_info_t__lockgroup_info__defined
#define	__NDR_convert__int_rep__Reply__host_lockgroup_info_t__lockgroup_info(a, f, c) \
	__NDR_convert__int_rep__lockgroup_info_array_t((lockgroup_info_array_t *)(a), f, c)
#elif	defined(__NDR_convert__int_rep__mach_host__lockgroup_info_t__defined)
#define	__NDR_convert__int_rep__Reply__host_lockgroup_info_t__lockgroup_info__defined
#define	__NDR_convert__int_rep__Reply__host_lockgroup_info_t__lockgroup_info(a, f, c) \
	__NDR_convert__ARRAY((lockgroup_info_t *)(a), f, c, __NDR_convert__int_rep__mach_host__lockgroup_info_t)
#elif	defined(__NDR_convert__int_rep__lockgroup_info_t__defined)
#define	__NDR_convert__int_rep__Reply__host_lockgroup_info_t__lockgroup_info__defined
#define	__NDR_convert__int_rep__Reply__host_lockgroup_info_t__lockgroup_info(a, f, c) \
	__NDR_convert__ARRAY((lockgroup_info_t *)(a), f, c, __NDR_convert__int_rep__lockgroup_info_t)
#elif	defined(__NDR_convert__int_rep__mach_host__integer_t__defined)
#define	__NDR_convert__int_rep__Reply__host_lockgroup_info_t__lockgroup_info__defined
#define	__NDR_convert__int_rep__Reply__host_lockgroup_info_t__lockgroup_info(a, f, c) \
	__NDR_convert__ARRAY((integer_t *)(a), f, 63 * (c), __NDR_convert__int_rep__mach_host__integer_t)
#elif	defined(__NDR_convert__int_rep__integer_t__defined)
#define	__NDR_convert__int_rep__Reply__host_lockgroup_info_t__lockgroup_info__defined
#define	__NDR_convert__int_rep__Reply__host_lockgroup_info_t__lockgroup_info(a, f, c) \
	__NDR_convert__ARRAY((integer_t *)(a), f, 63 * (c), __NDR_convert__int_rep__integer_t)
#elif	defined(__NDR_convert__int_rep__mach_host__int32_t__defined)
#define	__NDR_convert__int_rep__Reply__host_lockgroup_info_t__lockgroup_info__defined
#define	__NDR_convert__int_rep__Reply__host_lockgroup_info_t__lockgroup_info(a, f, c) \
	__NDR_convert__ARRAY((int32_t *)(a), f, 63 * (c), __NDR_convert__int_rep__mach_host__int32_t)
#elif	defined(__NDR_convert__int_rep__int32_t__defined)
#define	__NDR_convert__int_rep__Reply__host_lockgroup_info_t__lockgroup_info__defined
#define	__NDR_convert__int_rep__Reply__host_lockgroup_info_t__lockgroup_info(a, f, c) \
	__NDR_convert__ARRAY((int32_t *)(a), f, 63 * (c), __NDR_convert__int_rep__int32_t)
#endif /* defined(__NDR_convert__*__defined) */
#endif /* __NDR_convert__int_rep__Reply__host_lockgroup_info_t__lockgroup_info__defined */


#ifndef __NDR_convert__int_rep__Reply__host_lockgroup_info_t__lockgroup_infoCnt__defined
#if	defined(__NDR_convert__int_rep__mach_host__mach_msg_type_number_t__defined)
#define	__NDR_convert__int_rep__Reply__host_lockgroup_info_t__lockgroup_infoCnt__defined
#define	__NDR_convert__int_rep__Reply__host_lockgroup_info_t__lockgroup_infoCnt(a, f) \
	__NDR_convert__int_rep__mach_host__mach_msg_type_number_t((mach_msg_type_number_t *)(a), f)
#elif	defined(__NDR_convert__int_rep__mach_msg_type_number_t__defined)
#define	__NDR_convert__int_rep__Reply__host_lockgroup_info_t__lockgroup_infoCnt__defined
#define	__NDR_convert__int_rep__Reply__host_lockgroup_info_t__lockgroup_infoCnt(a, f) \
	__NDR_convert__int_rep__mach_msg_type_number_t((mach_msg_type_number_t *)(a), f)
#endif /* defined(__NDR_convert__*__defined) */
#endif /* __NDR_convert__int_rep__Reply__host_lockgroup_info_t__lockgroup_infoCnt__defined */


#ifndef __NDR_convert__char_rep__Reply__host_lockgroup_info_t__lockgroup_info__defined
#if	defined(__NDR_convert__char_rep__mach_host__lockgroup_info_array_t__defined)
#define	__NDR_convert__char_rep__Reply__host_lockgroup_info_t__lockgroup_info__defined
#define	__NDR_convert__char_rep__Reply__host_lockgroup_info_t__lockgroup_info(a, f, c) \
	__NDR_convert__char_rep__mach_host__lockgroup_info_array_t((lockgroup_info_array_t *)(a), f, c)
#elif	defined(__NDR_convert__char_rep__lockgroup_info_array_t__defined)
#define	__NDR_convert__char_rep__Reply__host_lockgroup_info_t__lockgroup_info__defined
#define	__NDR_convert__char_rep__Reply__host_lockgroup_info_t__lockgroup_info(a, f, c) \
	__NDR_convert__char_rep__lockgroup_info_array_t((lockgroup_info_array_t *)(a), f, c)
#elif	defined(__NDR_convert__char_rep__mach_host__lockgroup_info_t__defined)
#define	__NDR_convert__char_rep__Reply__host_lockgroup_info_t__lockgroup_info__defined
#define	__NDR_convert__char_rep__Reply__host_lockgroup_info_t__lockgroup_info(a, f, c) \
	__NDR_convert__ARRAY((lockgroup_info_t *)(a), f, c, __NDR_convert__char_rep__mach_host__lockgroup_info_t)
#elif	defined(__NDR_convert__char_rep__lockgroup_info_t__defined)
#define	__NDR_convert__char_rep__Reply__host_lockgroup_info_t__lockgroup_info__defined
#define	__NDR_convert__char_rep__Reply__host_lockgroup_info_t__lockgroup_info(a, f, c) \
	__NDR_convert__ARRAY((lockgroup_info_t *)(a), f, c, __NDR_convert__char_rep__lockgroup_info_t)
#elif	defined(__NDR_convert__char_rep__mach_host__integer_t__defined)
#define	__NDR_convert__char_rep__Reply__host_lockgroup_info_t__lockgroup_info__defined
#define	__NDR_convert__char_rep__Reply__host_lockgroup_info_t__lockgroup_info(a, f, c) \
	__NDR_convert__ARRAY((integer_t *)(a), f, 63 * (c), __NDR_convert__char_rep__mach_host__integer_t)
#elif	defined(__NDR_convert__char_rep__integer_t__defined)
#define	__NDR_convert__char_rep__Reply__host_lockgroup_info_t__lockgroup_info__defined
#define	__NDR_convert__char_rep__Reply__host_lockgroup_info_t__lockgroup_info(a, f, c) \
	__NDR_convert__ARRAY((integer_t *)(a), f, 63 * (c), __NDR_convert__char_rep__integer_t)
#elif	defined(__NDR_convert__char_rep__mach_host__int32_t__defined)
#define	__NDR_convert__char_rep__Reply__host_lockgroup_info_t__lockgroup_info__defined
#define	__NDR_convert__char_rep__Reply__host_lockgroup_info_t__lockgroup_info(a, f, c) \
	__NDR_convert__ARRAY((int32_t *)(a), f, 63 * (c), __NDR_convert__char_rep__mach_host__int32_t)
#elif	defined(__NDR_convert__char_rep__int32_t__defined)
#define	__NDR_convert__char_rep__Reply__host_lockgroup_info_t__lockgroup_info__defined
#define	__NDR_convert__char_rep__Reply__host_lockgroup_info_t__lockgroup_info(a, f, c) \
	__NDR_convert__ARRAY((int32_t *)(a), f, 63 * (c), __NDR_convert__char_rep__int32_t)
#endif /* defined(__NDR_convert__*__defined) */
#endif /* __NDR_convert__char_rep__Reply__host_lockgroup_info_t__lockgroup_info__defined */



#ifndef __NDR_convert__float_rep__Reply__host_lockgroup_info_t__lockgroup_info__defined
#if	defined(__NDR_convert__float_rep__mach_host__lockgroup_info_array_t__defined)
#define	__NDR_convert__float_rep__Reply__host_lockgroup_info_t__lockgroup_info__defined
#define	__NDR_convert__float_rep__Reply__host_lockgroup_info_t__lockgroup_info(a, f, c) \
	__NDR_convert__float_rep__mach_host__lockgroup_info_array_t((lockgroup_info_array_t *)(a), f, c)
#elif	defined(__NDR_convert__float_rep__lockgroup_info_array_t__defined)
#define	__NDR_convert__float_rep__Reply__host_lockgroup_info_t__lockgroup_info__defined
#define	__NDR_convert__float_rep__Reply__host_lockgroup_info_t__lockgroup_info(a, f, c) \
	__NDR_convert__float_rep__lockgroup_info_array_t((lockgroup_info_array_t *)(a), f, c)
#elif	defined(__NDR_convert__float_rep__mach_host__lockgroup_info_t__defined)
#define	__NDR_convert__float_rep__Reply__host_lockgroup_info_t__lockgroup_info__defined
#define	__NDR_convert__float_rep__Reply__host_lockgroup_info_t__lockgroup_info(a, f, c) \
	__NDR_convert__ARRAY((lockgroup_info_t *)(a), f, c, __NDR_convert__float_rep__mach_host__lockgroup_info_t)
#elif	defined(__NDR_convert__float_rep__lockgroup_info_t__defined)
#define	__NDR_convert__float_rep__Reply__host_lockgroup_info_t__lockgroup_info__defined
#define	__NDR_convert__float_rep__Reply__host_lockgroup_info_t__lockgroup_info(a, f, c) \
	__NDR_convert__ARRAY((lockgroup_info_t *)(a), f, c, __NDR_convert__float_rep__lockgroup_info_t)
#elif	defined(__NDR_convert__float_rep__mach_host__integer_t__defined)
#define	__NDR_convert__float_rep__Reply__host_lockgroup_info_t__lockgroup_info__defined
#define	__NDR_convert__float_rep__Reply__host_lockgroup_info_t__lockgroup_info(a, f, c) \
	__NDR_convert__ARRAY((integer_t *)(a), f, 63 * (c), __NDR_convert__float_rep__mach_host__integer_t)
#elif	defined(__NDR_convert__float_rep__integer_t__defined)
#define	__NDR_convert__float_rep__Reply__host_lockgroup_info_t__lockgroup_info__defined
#define	__NDR_convert__float_rep__Reply__host_lockgroup_info_t__lockgroup_info(a, f, c) \
	__NDR_convert__ARRAY((integer_t *)(a), f, 63 * (c), __NDR_convert__float_rep__integer_t)
#elif	defined(__NDR_convert__float_rep__mach_host__int32_t__defined)
#define	__NDR_convert__float_rep__Reply__host_lockgroup_info_t__lockgroup_info__defined
#define	__NDR_convert__float_rep__Reply__host_lockgroup_info_t__lockgroup_info(a, f, c) \
	__NDR_convert__ARRAY((int32_t *)(a), f, 63 * (c), __NDR_convert__float_rep__mach_host__int32_t)
#elif	defined(__NDR_convert__float_rep__int32_t__defined)
#define	__NDR_convert__float_rep__Reply__host_lockgroup_info_t__lockgroup_info__defined
#define	__NDR_convert__float_rep__Reply__host_lockgroup_info_t__lockgroup_info(a, f, c) \
	__NDR_convert__ARRAY((int32_t *)(a), f, 63 * (c), __NDR_convert__float_rep__int32_t)
#endif /* defined(__NDR_convert__*__defined) */
#endif /* __NDR_convert__float_rep__Reply__host_lockgroup_info_t__lockgroup_info__defined */




mig_internal kern_return_t __MIG_check__Reply__host_lockgroup_info_t(__Reply__host_lockgroup_info_t *Out0P)
{

	typedef __Reply__host_lockgroup_info_t __Reply;
	boolean_t msgh_simple;
#if	__MigTypeCheck
	unsigned int msgh_size;
#endif	/* __MigTypeCheck */
	if (Out0P->Head.msgh_id != 318) {
	    if (Out0P->Head.msgh_id == MACH_NOTIFY_SEND_ONCE)
		{ return MIG_SERVER_DIED; }
	    else
		{ return MIG_REPLY_MISMATCH; }
	}

	msgh_simple = !(Out0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX);
#if	__MigTypeCheck
	msgh_size = Out0P->Head.msgh_size;

	if ((msgh_simple || Out0P->msgh_body.msgh_descriptor_count != 1 ||
	    msgh_size != (mach_msg_size_t)sizeof(__Reply)) &&
	    (!msgh_simple || msgh_size != (mach_msg_size_t)sizeof(mig_reply_error_t) ||
	    ((mig_reply_error_t *)Out0P)->RetCode == KERN_SUCCESS))
		{ return MIG_TYPE_ERROR ; }
#endif	/* __MigTypeCheck */

	if (msgh_simple) {
#ifdef	__NDR_convert__mig_reply_error_t__defined
		__NDR_convert__mig_reply_error_t((mig_reply_error_t *)Out0P);
#endif	/* __NDR_convert__mig_reply_error_t__defined */
		return ((mig_reply_error_t *)Out0P)->RetCode;
	}

#if	__MigTypeCheck
	if (Out0P->lockgroup_info.type != MACH_MSG_OOL_DESCRIPTOR) {
		return MIG_TYPE_ERROR;
	}
#endif	/* __MigTypeCheck */

#if	defined(__NDR_convert__int_rep__Reply__host_lockgroup_info_t__lockgroup_info__defined) || \
	defined(__NDR_convert__int_rep__Reply__host_lockgroup_info_t__lockgroup_infoCnt__defined)
	if (Out0P->NDR.int_rep != NDR_record.int_rep) {
#if defined(__NDR_convert__int_rep__Reply__host_lockgroup_info_t__lockgroup_infoCnt__defined)
		__NDR_convert__int_rep__Reply__host_lockgroup_info_t__lockgroup_infoCnt(&Out0P->lockgroup_infoCnt, Out0P->NDR.int_rep);
#endif	/* __NDR_convert__int_rep__Reply__host_lockgroup_info_t__lockgroup_infoCnt__defined */
#if defined(__NDR_convert__int_rep__Reply__host_lockgroup_info_t__lockgroup_info__defined)
		__NDR_convert__int_rep__Reply__host_lockgroup_info_t__lockgroup_info((lockgroup_info_array_t)(Out0P->lockgroup_info.address), Out0P->NDR.int_rep, Out0P->lockgroup_infoCnt);
#endif /* __NDR_convert__int_rep__Reply__host_lockgroup_info_t__lockgroup_info__defined */
	}
#endif	/* defined(__NDR_convert__int_rep...) */

#if	defined(__NDR_convert__char_rep__Reply__host_lockgroup_info_t__lockgroup_info__defined) || \
	0
	if (Out0P->NDR.char_rep != NDR_record.char_rep) {
#if defined(__NDR_convert__char_rep__Reply__host_lockgroup_info_t__lockgroup_info__defined)
		__NDR_convert__char_rep__Reply__host_lockgroup_info_t__lockgroup_info((lockgroup_info_array_t)(Out0P->lockgroup_info.address), Out0P->NDR.char_rep, Out0P->lockgroup_infoCnt);
#endif /* __NDR_convert__char_rep__Reply__host_lockgroup_info_t__lockgroup_info__defined */
	}
#endif	/* defined(__NDR_convert__char_rep...) */

#if	defined(__NDR_convert__float_rep__Reply__host_lockgroup_info_t__lockgroup_info__defined) || \
	0
	if (Out0P->NDR.float_rep != NDR_record.float_rep) {
#if defined(__NDR_convert__float_rep__Reply__host_lockgroup_info_t__lockgroup_info__defined)
		__NDR_convert__float_rep__Reply__host_lockgroup_info_t__lockgroup_info((lockgroup_info_array_t)(Out0P->lockgroup_info.address), Out0P->NDR.float_rep, Out0P->lockgroup_infoCnt);
#endif /* __NDR_convert__float_rep__Reply__host_lockgroup_info_t__lockgroup_info__defined */
	}
#endif	/* defined(__NDR_convert__float_rep...) */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Reply__host_lockgroup_info_t__defined) */
#endif /* __MIG_check__Reply__mach_host_subsystem__ */
#endif /* ( __MigTypeCheck || __NDR_convert__ ) */


/* Routine host_lockgroup_info */
mig_external kern_return_t host_lockgroup_info
(
	host_t host,
	lockgroup_info_array_t *lockgroup_info,
	mach_msg_type_number_t *lockgroup_infoCnt
)
{

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
	} Request;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		mach_msg_ool_descriptor_t lockgroup_info;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		mach_msg_type_number_t lockgroup_infoCnt;
		mach_msg_trailer_t trailer;
	} Reply;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		mach_msg_ool_descriptor_t lockgroup_info;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		mach_msg_type_number_t lockgroup_infoCnt;
	} __Reply;
#ifdef  __MigPackStructs
#pragma pack()
#endif
	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	union {
		Request In;
		Reply Out;
	} Mess;

	Request *InP = &Mess.In;
	Reply *Out0P = &Mess.Out;

	mach_msg_return_t msg_result;

#ifdef	__MIG_check__Reply__host_lockgroup_info_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Reply__host_lockgroup_info_t__defined */

	__DeclareSendRpc(218, "host_lockgroup_info")

	InP->Head.msgh_bits =
		MACH_MSGH_BITS(19, MACH_MSG_TYPE_MAKE_SEND_ONCE);
	/* msgh_size passed as argument */
	InP->Head.msgh_request_port = host;
	InP->Head.msgh_reply_port = mig_get_reply_port();
	InP->Head.msgh_id = 218;

	__BeforeSendRpc(218, "host_lockgroup_info")
	msg_result = mach_msg(&InP->Head, MACH_SEND_MSG|MACH_RCV_MSG|MACH_MSG_OPTION_NONE, (mach_msg_size_t)sizeof(Request), (mach_msg_size_t)sizeof(Reply), InP->Head.msgh_reply_port, MACH_MSG_TIMEOUT_NONE, MACH_PORT_NULL);
	__AfterSendRpc(218, "host_lockgroup_info")
	if (msg_result != MACH_MSG_SUCCESS) {
		__MachMsgErrorWithoutTimeout(msg_result);
		{ return msg_result; }
	}


#if	defined(__MIG_check__Reply__host_lockgroup_info_t__defined)
	check_result = __MIG_check__Reply__host_lockgroup_info_t((__Reply__host_lockgroup_info_t *)Out0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ return check_result; }
#endif	/* defined(__MIG_check__Reply__host_lockgroup_info_t__defined) */

	*lockgroup_info = (lockgroup_info_array_t)(Out0P->lockgroup_info.address);
	*lockgroup_infoCnt = Out0P->lockgroup_infoCnt;

	return KERN_SUCCESS;
}

#if ( __MigTypeCheck || __NDR_convert__ )
#if __MIG_check__Reply__mach_host_subsystem__
#if !defined(__MIG_check__Reply__host_statistics64_t__defined)
#define __MIG_check__Reply__host_statistics64_t__defined
#ifndef __NDR_convert__int_rep__Reply__host_statistics64_t__RetCode__defined
#if	defined(__NDR_convert__int_rep__mach_host__kern_return_t__defined)
#define	__NDR_convert__int_rep__Reply__host_statistics64_t__RetCode__defined
#define	__NDR_convert__int_rep__Reply__host_statistics64_t__RetCode(a, f) \
	__NDR_convert__int_rep__mach_host__kern_return_t((kern_return_t *)(a), f)
#elif	defined(__NDR_convert__int_rep__kern_return_t__defined)
#define	__NDR_convert__int_rep__Reply__host_statistics64_t__RetCode__defined
#define	__NDR_convert__int_rep__Reply__host_statistics64_t__RetCode(a, f) \
	__NDR_convert__int_rep__kern_return_t((kern_return_t *)(a), f)
#endif /* defined(__NDR_convert__*__defined) */
#endif /* __NDR_convert__int_rep__Reply__host_statistics64_t__RetCode__defined */


#ifndef __NDR_convert__int_rep__Reply__host_statistics64_t__host_info64_out__defined
#if	defined(__NDR_convert__int_rep__mach_host__host_info64_t__defined)
#define	__NDR_convert__int_rep__Reply__host_statistics64_t__host_info64_out__defined
#define	__NDR_convert__int_rep__Reply__host_statistics64_t__host_info64_out(a, f, c) \
	__NDR_convert__int_rep__mach_host__host_info64_t((host_info64_t *)(a), f, c)
#elif	defined(__NDR_convert__int_rep__host_info64_t__defined)
#define	__NDR_convert__int_rep__Reply__host_statistics64_t__host_info64_out__defined
#define	__NDR_convert__int_rep__Reply__host_statistics64_t__host_info64_out(a, f, c) \
	__NDR_convert__int_rep__host_info64_t((host_info64_t *)(a), f, c)
#elif	defined(__NDR_convert__int_rep__mach_host__integer_t__defined)
#define	__NDR_convert__int_rep__Reply__host_statistics64_t__host_info64_out__defined
#define	__NDR_convert__int_rep__Reply__host_statistics64_t__host_info64_out(a, f, c) \
	__NDR_convert__ARRAY((integer_t *)(a), f, c, __NDR_convert__int_rep__mach_host__integer_t)
#elif	defined(__NDR_convert__int_rep__integer_t__defined)
#define	__NDR_convert__int_rep__Reply__host_statistics64_t__host_info64_out__defined
#define	__NDR_convert__int_rep__Reply__host_statistics64_t__host_info64_out(a, f, c) \
	__NDR_convert__ARRAY((integer_t *)(a), f, c, __NDR_convert__int_rep__integer_t)
#elif	defined(__NDR_convert__int_rep__mach_host__int32_t__defined)
#define	__NDR_convert__int_rep__Reply__host_statistics64_t__host_info64_out__defined
#define	__NDR_convert__int_rep__Reply__host_statistics64_t__host_info64_out(a, f, c) \
	__NDR_convert__ARRAY((int32_t *)(a), f, c, __NDR_convert__int_rep__mach_host__int32_t)
#elif	defined(__NDR_convert__int_rep__int32_t__defined)
#define	__NDR_convert__int_rep__Reply__host_statistics64_t__host_info64_out__defined
#define	__NDR_convert__int_rep__Reply__host_statistics64_t__host_info64_out(a, f, c) \
	__NDR_convert__ARRAY((int32_t *)(a), f, c, __NDR_convert__int_rep__int32_t)
#endif /* defined(__NDR_convert__*__defined) */
#endif /* __NDR_convert__int_rep__Reply__host_statistics64_t__host_info64_out__defined */


#ifndef __NDR_convert__int_rep__Reply__host_statistics64_t__host_info64_outCnt__defined
#if	defined(__NDR_convert__int_rep__mach_host__mach_msg_type_number_t__defined)
#define	__NDR_convert__int_rep__Reply__host_statistics64_t__host_info64_outCnt__defined
#define	__NDR_convert__int_rep__Reply__host_statistics64_t__host_info64_outCnt(a, f) \
	__NDR_convert__int_rep__mach_host__mach_msg_type_number_t((mach_msg_type_number_t *)(a), f)
#elif	defined(__NDR_convert__int_rep__mach_msg_type_number_t__defined)
#define	__NDR_convert__int_rep__Reply__host_statistics64_t__host_info64_outCnt__defined
#define	__NDR_convert__int_rep__Reply__host_statistics64_t__host_info64_outCnt(a, f) \
	__NDR_convert__int_rep__mach_msg_type_number_t((mach_msg_type_number_t *)(a), f)
#endif /* defined(__NDR_convert__*__defined) */
#endif /* __NDR_convert__int_rep__Reply__host_statistics64_t__host_info64_outCnt__defined */



#ifndef __NDR_convert__char_rep__Reply__host_statistics64_t__host_info64_out__defined
#if	defined(__NDR_convert__char_rep__mach_host__host_info64_t__defined)
#define	__NDR_convert__char_rep__Reply__host_statistics64_t__host_info64_out__defined
#define	__NDR_convert__char_rep__Reply__host_statistics64_t__host_info64_out(a, f, c) \
	__NDR_convert__char_rep__mach_host__host_info64_t((host_info64_t *)(a), f, c)
#elif	defined(__NDR_convert__char_rep__host_info64_t__defined)
#define	__NDR_convert__char_rep__Reply__host_statistics64_t__host_info64_out__defined
#define	__NDR_convert__char_rep__Reply__host_statistics64_t__host_info64_out(a, f, c) \
	__NDR_convert__char_rep__host_info64_t((host_info64_t *)(a), f, c)
#elif	defined(__NDR_convert__char_rep__mach_host__integer_t__defined)
#define	__NDR_convert__char_rep__Reply__host_statistics64_t__host_info64_out__defined
#define	__NDR_convert__char_rep__Reply__host_statistics64_t__host_info64_out(a, f, c) \
	__NDR_convert__ARRAY((integer_t *)(a), f, c, __NDR_convert__char_rep__mach_host__integer_t)
#elif	defined(__NDR_convert__char_rep__integer_t__defined)
#define	__NDR_convert__char_rep__Reply__host_statistics64_t__host_info64_out__defined
#define	__NDR_convert__char_rep__Reply__host_statistics64_t__host_info64_out(a, f, c) \
	__NDR_convert__ARRAY((integer_t *)(a), f, c, __NDR_convert__char_rep__integer_t)
#elif	defined(__NDR_convert__char_rep__mach_host__int32_t__defined)
#define	__NDR_convert__char_rep__Reply__host_statistics64_t__host_info64_out__defined
#define	__NDR_convert__char_rep__Reply__host_statistics64_t__host_info64_out(a, f, c) \
	__NDR_convert__ARRAY((int32_t *)(a), f, c, __NDR_convert__char_rep__mach_host__int32_t)
#elif	defined(__NDR_convert__char_rep__int32_t__defined)
#define	__NDR_convert__char_rep__Reply__host_statistics64_t__host_info64_out__defined
#define	__NDR_convert__char_rep__Reply__host_statistics64_t__host_info64_out(a, f, c) \
	__NDR_convert__ARRAY((int32_t *)(a), f, c, __NDR_convert__char_rep__int32_t)
#endif /* defined(__NDR_convert__*__defined) */
#endif /* __NDR_convert__char_rep__Reply__host_statistics64_t__host_info64_out__defined */




#ifndef __NDR_convert__float_rep__Reply__host_statistics64_t__host_info64_out__defined
#if	defined(__NDR_convert__float_rep__mach_host__host_info64_t__defined)
#define	__NDR_convert__float_rep__Reply__host_statistics64_t__host_info64_out__defined
#define	__NDR_convert__float_rep__Reply__host_statistics64_t__host_info64_out(a, f, c) \
	__NDR_convert__float_rep__mach_host__host_info64_t((host_info64_t *)(a), f, c)
#elif	defined(__NDR_convert__float_rep__host_info64_t__defined)
#define	__NDR_convert__float_rep__Reply__host_statistics64_t__host_info64_out__defined
#define	__NDR_convert__float_rep__Reply__host_statistics64_t__host_info64_out(a, f, c) \
	__NDR_convert__float_rep__host_info64_t((host_info64_t *)(a), f, c)
#elif	defined(__NDR_convert__float_rep__mach_host__integer_t__defined)
#define	__NDR_convert__float_rep__Reply__host_statistics64_t__host_info64_out__defined
#define	__NDR_convert__float_rep__Reply__host_statistics64_t__host_info64_out(a, f, c) \
	__NDR_convert__ARRAY((integer_t *)(a), f, c, __NDR_convert__float_rep__mach_host__integer_t)
#elif	defined(__NDR_convert__float_rep__integer_t__defined)
#define	__NDR_convert__float_rep__Reply__host_statistics64_t__host_info64_out__defined
#define	__NDR_convert__float_rep__Reply__host_statistics64_t__host_info64_out(a, f, c) \
	__NDR_convert__ARRAY((integer_t *)(a), f, c, __NDR_convert__float_rep__integer_t)
#elif	defined(__NDR_convert__float_rep__mach_host__int32_t__defined)
#define	__NDR_convert__float_rep__Reply__host_statistics64_t__host_info64_out__defined
#define	__NDR_convert__float_rep__Reply__host_statistics64_t__host_info64_out(a, f, c) \
	__NDR_convert__ARRAY((int32_t *)(a), f, c, __NDR_convert__float_rep__mach_host__int32_t)
#elif	defined(__NDR_convert__float_rep__int32_t__defined)
#define	__NDR_convert__float_rep__Reply__host_statistics64_t__host_info64_out__defined
#define	__NDR_convert__float_rep__Reply__host_statistics64_t__host_info64_out(a, f, c) \
	__NDR_convert__ARRAY((int32_t *)(a), f, c, __NDR_convert__float_rep__int32_t)
#endif /* defined(__NDR_convert__*__defined) */
#endif /* __NDR_convert__float_rep__Reply__host_statistics64_t__host_info64_out__defined */




mig_internal kern_return_t __MIG_check__Reply__host_statistics64_t(__Reply__host_statistics64_t *Out0P)
{

	typedef __Reply__host_statistics64_t __Reply;
#if	__MigTypeCheck
	unsigned int msgh_size;
#endif	/* __MigTypeCheck */

	if (Out0P->Head.msgh_id != 319) {
	    if (Out0P->Head.msgh_id == MACH_NOTIFY_SEND_ONCE)
		{ return MIG_SERVER_DIED; }
	    else
		{ return MIG_REPLY_MISMATCH; }
	}

#if	__MigTypeCheck
	msgh_size = Out0P->Head.msgh_size;

	if ((Out0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) ||
	    ((msgh_size > (mach_msg_size_t)sizeof(__Reply) || msgh_size < (mach_msg_size_t)(sizeof(__Reply) - 1024)) &&
	     (msgh_size != (mach_msg_size_t)sizeof(mig_reply_error_t) ||
	      Out0P->RetCode == KERN_SUCCESS)))
		{ return MIG_TYPE_ERROR ; }
#endif	/* __MigTypeCheck */

	if (Out0P->RetCode != KERN_SUCCESS) {
#ifdef	__NDR_convert__mig_reply_error_t__defined
		__NDR_convert__mig_reply_error_t((mig_reply_error_t *)Out0P);
#endif	/* __NDR_convert__mig_reply_error_t__defined */
		return ((mig_reply_error_t *)Out0P)->RetCode;
	}

#if defined(__NDR_convert__int_rep__Reply__host_statistics64_t__host_info64_outCnt__defined)
	if (Out0P->NDR.int_rep != NDR_record.int_rep)
		__NDR_convert__int_rep__Reply__host_statistics64_t__host_info64_outCnt(&Out0P->host_info64_outCnt, Out0P->NDR.int_rep);
#endif	/* __NDR_convert__int_rep__Reply__host_statistics64_t__host_info64_outCnt__defined */
#if	__MigTypeCheck
	if (msgh_size != (mach_msg_size_t)(sizeof(__Reply) - 1024) + ((4 * Out0P->host_info64_outCnt)))
		{ return MIG_TYPE_ERROR ; }
#endif	/* __MigTypeCheck */

#if	defined(__NDR_convert__int_rep__Reply__host_statistics64_t__RetCode__defined) || \
	defined(__NDR_convert__int_rep__Reply__host_statistics64_t__host_info64_out__defined) || \
	defined(__NDR_convert__int_rep__Reply__host_statistics64_t__host_info64_outCnt__defined)
	if (Out0P->NDR.int_rep != NDR_record.int_rep) {
#if defined(__NDR_convert__int_rep__Reply__host_statistics64_t__RetCode__defined)
		__NDR_convert__int_rep__Reply__host_statistics64_t__RetCode(&Out0P->RetCode, Out0P->NDR.int_rep);
#endif /* __NDR_convert__int_rep__Reply__host_statistics64_t__RetCode__defined */
#if defined(__NDR_convert__int_rep__Reply__host_statistics64_t__host_info64_out__defined)
		__NDR_convert__int_rep__Reply__host_statistics64_t__host_info64_out(&Out0P->host_info64_out, Out0P->NDR.int_rep, Out0P->host_info64_outCnt);
#endif /* __NDR_convert__int_rep__Reply__host_statistics64_t__host_info64_out__defined */
	}
#endif	/* defined(__NDR_convert__int_rep...) */

#if	0 || \
	defined(__NDR_convert__char_rep__Reply__host_statistics64_t__host_info64_out__defined) || \
	0
	if (Out0P->NDR.char_rep != NDR_record.char_rep) {
#if defined(__NDR_convert__char_rep__Reply__host_statistics64_t__host_info64_out__defined)
		__NDR_convert__char_rep__Reply__host_statistics64_t__host_info64_out(&Out0P->host_info64_out, Out0P->NDR.char_rep, Out0P->host_info64_outCnt);
#endif /* __NDR_convert__char_rep__Reply__host_statistics64_t__host_info64_out__defined */
	}
#endif	/* defined(__NDR_convert__char_rep...) */

#if	0 || \
	defined(__NDR_convert__float_rep__Reply__host_statistics64_t__host_info64_out__defined) || \
	0
	if (Out0P->NDR.float_rep != NDR_record.float_rep) {
#if defined(__NDR_convert__float_rep__Reply__host_statistics64_t__host_info64_out__defined)
		__NDR_convert__float_rep__Reply__host_statistics64_t__host_info64_out(&Out0P->host_info64_out, Out0P->NDR.float_rep, Out0P->host_info64_outCnt);
#endif /* __NDR_convert__float_rep__Reply__host_statistics64_t__host_info64_out__defined */
	}
#endif	/* defined(__NDR_convert__float_rep...) */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Reply__host_statistics64_t__defined) */
#endif /* __MIG_check__Reply__mach_host_subsystem__ */
#endif /* ( __MigTypeCheck || __NDR_convert__ ) */


/* Routine host_statistics64 */
mig_external kern_return_t host_statistics64
(
	host_t host_priv,
	host_flavor_t flavor,
	host_info64_t host_info64_out,
	mach_msg_type_number_t *host_info64_outCnt
)
{

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		NDR_record_t NDR;
		host_flavor_t flavor;
		mach_msg_type_number_t host_info64_outCnt;
	} Request;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		NDR_record_t NDR;
		kern_return_t RetCode;
		mach_msg_type_number_t host_info64_outCnt;
		integer_t host_info64_out[256];
		mach_msg_trailer_t trailer;
	} Reply;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		NDR_record_t NDR;
		kern_return_t RetCode;
		mach_msg_type_number_t host_info64_outCnt;
		integer_t host_info64_out[256];
	} __Reply;
#ifdef  __MigPackStructs
#pragma pack()
#endif
	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	union {
		Request In;
		Reply Out;
	} Mess;

	Request *InP = &Mess.In;
	Reply *Out0P = &Mess.Out;

	mach_msg_return_t msg_result;

#ifdef	__MIG_check__Reply__host_statistics64_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Reply__host_statistics64_t__defined */

	__DeclareSendRpc(219, "host_statistics64")

	InP->NDR = NDR_record;

	InP->flavor = flavor;

	if (*host_info64_outCnt < 256)
		InP->host_info64_outCnt = *host_info64_outCnt;
	else
		InP->host_info64_outCnt = 256;

	InP->Head.msgh_bits =
		MACH_MSGH_BITS(19, MACH_MSG_TYPE_MAKE_SEND_ONCE);
	/* msgh_size passed as argument */
	InP->Head.msgh_request_port = host_priv;
	InP->Head.msgh_reply_port = mig_get_reply_port();
	InP->Head.msgh_id = 219;

	__BeforeSendRpc(219, "host_statistics64")
	msg_result = mach_msg(&InP->Head, MACH_SEND_MSG|MACH_RCV_MSG|MACH_MSG_OPTION_NONE, (mach_msg_size_t)sizeof(Request), (mach_msg_size_t)sizeof(Reply), InP->Head.msgh_reply_port, MACH_MSG_TIMEOUT_NONE, MACH_PORT_NULL);
	__AfterSendRpc(219, "host_statistics64")
	if (msg_result != MACH_MSG_SUCCESS) {
		__MachMsgErrorWithoutTimeout(msg_result);
		{ return msg_result; }
	}


#if	defined(__MIG_check__Reply__host_statistics64_t__defined)
	check_result = __MIG_check__Reply__host_statistics64_t((__Reply__host_statistics64_t *)Out0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ return check_result; }
#endif	/* defined(__MIG_check__Reply__host_statistics64_t__defined) */

	if (Out0P->host_info64_outCnt > *host_info64_outCnt) {
		(void)memcpy((char *) host_info64_out, (const char *) Out0P->host_info64_out, 4 *  *host_info64_outCnt);
		*host_info64_outCnt = Out0P->host_info64_outCnt;
		{ return MIG_ARRAY_TOO_LARGE; }
	}
	(void)memcpy((char *) host_info64_out, (const char *) Out0P->host_info64_out, 4 * Out0P->host_info64_outCnt);

	*host_info64_outCnt = Out0P->host_info64_outCnt;

	return KERN_SUCCESS;
}
