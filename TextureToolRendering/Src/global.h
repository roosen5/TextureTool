#define FORWARD_DECLARE_CLASS(className) class className;

#define SAFE_DELETE(var) \
{\
	if (var != nullptr)  \
	{\
		delete var;\
		var = nullptr;\
	}\
}

#define SAFE_RELEASE(var) \
{ \
	if ( var!=nullptr )  \
	{ \
		var->Release(); \
		var = nullptr; \
	} \
}

#define E_UNSUPPORTEDFILE -0xFF00001