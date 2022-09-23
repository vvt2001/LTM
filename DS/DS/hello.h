/*file: hello.h */
/* this ALWAYS GENERATED file contains the definitions for the interfaces */
/* File created by MIDL compiler version 3.00.06
/* at Tue Feb 20 11:33:32 1996 */
/* Compiler settings for hello.idl:
Os, W1, Zp8, env=Win32, ms_ext, c_ext
error checks: none */
//@@MIDL_FILE_HEADING(  )
#include "Rpc.h"
#include "rpcndr.h"

#ifndef __hello_h_
#define __hello_h_

#ifdef __cplusplus
extern "C" {
#endif 

	/* Forward Declarations */

	void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
	void __RPC_USER MIDL_user_free(void __RPC_FAR *);

#ifndef __hello_INTERFACE_DEFINED__
#define __hello_INTERFACE_DEFINED__

	/****************************************
	* Generated header for interface: hello
	* at Tue Feb 20 11:33:32 1996
	* using MIDL 3.00.06
	****************************************/
	/* [implicit_handle][version][uuid] */

	/* size is 0 */
	void HelloProc(
		/* [string][in] */ unsigned char __RPC_FAR *pszString);
	/* size is 0 */
	void Shutdown(void);
	extern handle_t hello_IfHandle;

	extern RPC_IF_HANDLE hello_v1_0_c_ifspec;
	extern RPC_IF_HANDLE hello_v1_0_s_ifspec;
#endif /* __hello_INTERFACE_DEFINED__ */

	/* Additional Prototypes for ALL interfaces */
	/* end of Additional Prototypes */
#ifdef __cplusplus
}
#endif
#endif