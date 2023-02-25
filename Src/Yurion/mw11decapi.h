#ifndef _MW_DEC_API_H_
#define _MW_DEC_API_H_

#ifdef WIN32

#if defined(MW11DEC_BUILD_DLL)
#   ifdef mw11dec_EXPORTS
#       define MW11DEC_API __declspec(dllexport)
#   else
#       define MW11DEC_API __declspec(dllimport)
#   endif //mw11dec_EXPORTS
#else
#   define MW11DEC_API
#endif  //defined(MW11DEC_BUILD_DLL)

#else

#ifdef __cplusplus
#   define MW11DEC_API extern "C" 
#else
#   define MW11DEC_API
#endif //__cpluspluc

#endif //WIN32

#endif // _MW_DEC_API_H_
