RELEASE_FLAGS=/Ox
DEBUG_FLAGS=/RTCsu /DDEBUG
CXXFLAGS=/EHsc \
    /nologo /GS /Zc:forScope,auto /W4 /wd4482 /Dmd_md5 /DCURL_STATICLIB

# Make sure to use /nologo on recursive calls. Done this way because
# I can't find a way to overwrite MAKEFLAGS directly.
# Shorthand L is used in place of nologo because otherwise
# "nmake all" can't intepret the flags correctly.
MAKE_FLAGS=L$(MAKEFLAGS)

# MODE is set on the recursive call to Nmake. Its value is equal to
# the target and so is restricted to (debug, release).
SOURCES=src
SYMBOLS=symbols
OBJ_DIR=$(SYMBOLS)\$(MODE)
BINARY=bin
TARGET=$(BINARY)\$(MODE)\EfULauncher.exe
RESOURCES=res

OBJS=$(OBJ_DIR)\main.obj\
    $(OBJ_DIR)\curleasy.obj\
    $(OBJ_DIR)\win_error_string.obj\
    $(OBJ_DIR)\simple_read_hklm_key.obj\
    $(OBJ_DIR)\target.obj\
    $(OBJ_DIR)\efulauncher.obj

LDLIBS=libcurl_a.lib libeay32MD.lib

# Build targets. Default build is debug. Build targets are all recursive
# in order to setup correct directory structure and compile flags.
all:
    $(MAKE) /$(MAKE_FLAGS) debug

debug:
    $(MAKE) /$(MAKE_FLAGS) MODE=$(@) CXXFLAGS="$(CXXFLAGS) $(DEBUG_FLAGS)" \
        $(BINARY)\$(@)\EfULauncher.exe

release:
    $(MAKE) /$(MAKE_FLAGS) MODE=$(@) CXXFLAGS="$(CXXFLAGS) $(RELEASE_FLAGS)" \
        $(BINARY)\$(@)\EfULauncher.exe

clean:
    @ if exist $(SYMBOLS) rmdir /S /Q $(SYMBOLS)

clobber: clean
	@ if exist $(BINARY) rmdir /S /Q $(BINARY)

$(TARGET): $(OBJS)
    @ if not exist $(@D) mkdir $(@D)
    @ if exist $(RESOURCES) copy $(RESOURCES)\* $(@D)
    link /nologo /out:"$(TARGET)" $(OBJS) \
        /libpath:"..\curl\builds\libcurl-vc-x86-release-static-ipv6-sspi-winssl\lib" \
        /libpath:"D:\Programmer\OpenSSL-Win32\lib\VC" \
        $(LDLIBS)

{$(SOURCES)}.cpp{$(OBJ_DIR)}.obj:
    @ if not exist $(@D) mkdir $(@D)
    $(CXX) /c $(CXXFLAGS) /MD \
    /I"D:\Programmer\OpenSSL-Win32\include" \
    /I"..\curl\builds\libcurl-vc-x86-release-static-ipv6-sspi-winssl\include" \
    $(<) /Fo"$(@)"
