#define FORWARD_DECLARE_CLASS(className) class className
#define SAFE_RELEASE(var) \
{ \
	if ( var!=nullptr )  \
	{ \
		var->Release(); \
		var = 0; \
	} \
}