#!/bin/sh

mkdir MultiplexManager.app/Contents/Frameworks
cp -R ~/QtSDK/Desktop/Qt/472/gcc/lib/QtCore.framework MultiplexManager.app/Contents/Frameworks
cp -R ~/QtSDK/Desktop/Qt/472/gcc/lib/QtGui.framework MultiplexManager.app/Contents/Frameworks
cp -R ~/QtSDK/Desktop/Qt/472/gcc/lib/QtXml.framework MultiplexManager.app/Contents/Frameworks

install_name_tool -id @executable_path/../Frameworks/QtCore.framework/Versions/Current/QtCore MultiplexManager.app/Contents/Frameworks/QtCore.framework/Versions/Current/QtCore
install_name_tool -id @executable_path/../Frameworks/QtGui.framework/Versions/Current/QtGui MultiplexManager.app/Contents/Frameworks/QtGui.framework/Versions/Current/QtGui
install_name_tool -id @executable_path/../Frameworks/QtXml.framework/Versions/Current/QtXml MultiplexManager.app/Contents/Frameworks/QtXml.framework/Versions/Current/QtXml

install_name_tool -change /Users/paul/QtSDK/Desktop/Qt/472/gcc/lib/QtCore.framework/Versions/Current/QtCore @executable_path/../Frameworks/QtCore.framework/Versions/Current/QtCore MultiplexManager.app/Contents/MacOs/MultiplexManager
install_name_tool -change /Users/paul/QtSDK/Desktop/Qt/472/gcc/lib/QtGui.framework/Versions/Current/QtGui @executable_path/../Frameworks/QtGui.framework/Versions/Current/QtGui MultiplexManager.app/Contents/MacOs/MultiplexManager
install_name_tool -change /Users/paul/QtSDK/Desktop/Qt/472/gcc/lib/QtXml.framework/Versions/Current/QtXml @executable_path/../Frameworks/QtXml.framework/Versions/Current/QtXml MultiplexManager.app/Contents/MacOs/MultiplexManager

install_name_tool -change /Users/paul/QtSDK/Desktop/Qt/472/gcc/lib/QtCore.framework/Versions/4/QtCore @executable_path/../Frameworks/QtCore.framework/Versions/Current/QtCore MultiplexManager.app/Contents/Frameworks/QtGui.framework/Versions/Current/QtGui

install_name_tool -change /Users/paul/QtSDK/Desktop/Qt/472/gcc/lib/QtCore.framework/Versions/4/QtCore @executable_path/../Frameworks/QtCore.framework/Versions/Current/QtCore MultiplexManager.app/Contents/Frameworks/QtXml.framework/Versions/Current/QtXml

# clean up the debug libs and headers so the package isn't a million meg

find MultiplexManager.app/Contents/Frameworks/ | grep _debug | xargs rm -f
find MultiplexManager.app/Contents/Frameworks/ | grep Headers | xargs rm -f



# EXECFILE=MultiplexManager.app/Contents/MacOs/MultiplexManager
# LIBPATH=MultiplexManager.app/Contents/SharedSupport

# NEWLIBPATH="@executable_path/../SharedSupport"

# mkdir ${LIBPATH};

# # space separated list of libraries
# #TARGETS="libiconv.2.dylib libstdc++.6.dylib"
# TARGETS="libiconv.2.dylib"
# for TARGET in ${TARGETS} ; do
# cp /usr/lib/${TARGET} ${LIBPATH}
# LIBFILE=${LIBPATH}/${TARGET}
# TARGETID=`otool -DX ${LIBPATH}/$TARGET`
# NEWTARGETID=${NEWLIBPATH}/${TARGET}
# install_name_tool -id ${NEWTARGETID} ${LIBFILE}
# install_name_tool -change ${TARGETID} ${NEWTARGETID} ${EXECFILE}
# install_name_tool -change ${TARGETID} ${NEWTARGETID} MultiplexManager.app/Contents/Frameworks/QtCore.framework/Versions/Current/QtCore
# install_name_tool -change ${TARGETID} ${NEWTARGETID} MultiplexManager.app/Contents/Frameworks/QtGui.framework/Versions/Current/QtGui
# install_name_tool -change ${TARGETID} ${NEWTARGETID} MultiplexManager.app/Contents/Frameworks/QtXml.framework/Versions/Current/QtXml

# done