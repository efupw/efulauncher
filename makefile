RELEASE_FLAGS=/Ox
DEBUG_FLAGS=/RTCsu /DDEBUG
CXXFLAGS=/FIcpp11_compliance.h /EHsc \
    /nologo /GS /Zc:forScope,auto /W4 /wd4482 /Dmd_md5

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

OBJS=$(OBJ_DIR)\main.obj $(OBJ_DIR)\curleasy.obj
LDLIBS=curllib.lib libeay32MD.lib ssleay32MD.lib

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
    link /nologo /out:"$(TARGET)" $(OBJS) \
        /libpath:"D:\Programmer\libcurl-7.19.3\lib\Release" \
        /libpath:"D:\Programmer\OpenSSL-Win32\lib\VC" \
        $(LDLIBS)

{$(SOURCES)}.cpp{$(OBJ_DIR)}.obj:
    @ if not exist $(@D) mkdir $(@D)
    $(CXX) /c $(CXXFLAGS) \
    /I"D:\Programmer\OpenSSL-Win32\include" \
    /I"D:\Programmer\libcurl-7.19.3\include" \
    $(<) /Fo"$(@)"