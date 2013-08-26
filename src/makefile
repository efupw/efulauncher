TARGET=EfULauncher.exe
CXXFLAGS=/FI "D:/repos/efulauncher/cpp11_compliance.h" /EHsc /nologo /GS /RTCu /Zc:forScope,auto /W4 /Dmd_md5 /DDEBUG /DWIN
OBJS=main.obj curleasy.obj

all: $(TARGET)

clean:
	del *.obj core *.stackdump

clobber: clean
	del $(TARGET)

$(TARGET): $(OBJS)
    $(CXX) $(CXXFLAGS) /I"D:\Programmer\OpenSSL-Win32\include"  /I"D:\Programmer\libcurl-7.19.3\include" /Fe"$(TARGET)" $(OBJS) /link /LIBPATH:"D:\Programmer\libcurl-7.19.3\lib\Release" curllib.lib

.cpp.obj:
    $(CXX) /c $(CXXFLAGS) /I"D:\Programmer\OpenSSL-Win32\include"  /I"D:\Programmer\libcurl-7.19.3\include" $< /link /LIBPATH:"D:\Programmer\libcurl-7.19.3\lib\Release" curllib.lib


#_MSC_VER
# && (_MSC_FULL_VER >= 170050727)
#define CPP11_FOR_EACH
