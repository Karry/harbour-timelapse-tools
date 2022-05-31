Name:       harbour-timelapse-tools

# >> macros

# ignore installed files that are not packed to rpm
%define _unpackaged_files_terminate_build 0

# don't setup rpm provides
%define __provides_exclude_from ^%{_datadir}/.*$

# don't setup rpm requires
# list here all the libraries your RPM installs
%define __requires_exclude ^ld-linux|libMagick*|libgphoto2*|libtimelapse.so|libv4l*|libvidstab.so.*$

# << macros

Summary:    TimeLapse tools for Sailfish OS
Version:    0.1
Release:    1
Group:      Qt/Qt
License:    GPLv2
URL:        https://github.com/Karry/harbour-timelapse-tools
Source0:    %{name}-%{version}.tar.bz2
BuildRequires:  pkgconfig(Qt5Core)
BuildRequires:  pkgconfig(Qt5DBus)
BuildRequires:  pkgconfig(Qt5Multimedia)
BuildRequires:  pkgconfig(Qt5Qml)
BuildRequires:  pkgconfig(Qt5Quick)
BuildRequires:  pkgconfig(Qt5Svg)
BuildRequires:  pkgconfig(sailfishapp) >= 1.0.2
BuildRequires:  libtool-ltdl-devel
BuildRequires:  cmake
BuildRequires:  chrpath
BuildRequires:  desktop-file-utils
BuildRequires:  git
BuildRequires:  qt5-qttools-linguist

%description
TimeLapse tools is app for capturing and assembling timelapse videos.

%prep
%setup -q -n %{name}-%{version}

# >> setup
# << setup

%build
# >> build pre
#rm -rf rpmbuilddir-%{_arch}
mkdir -p rpmbuilddir-%{_arch}

## for production build:
cd rpmbuilddir-%{_arch} && cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo -DQT_QML_DEBUG=no -DSANITIZER=none -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_INSTALL_RPATH=%{_datadir}/%{name}/lib/: ..
## for debug build, use these cmake arguments instead:
#cd rpmbuilddir-%{_arch} && cmake -DCMAKE_BUILD_TYPE=DEBUG -DQT_QML_DEBUG=yes -DSANITIZER=none -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_INSTALL_RPATH=%{_datadir}/%{name}/lib/: ..

cd ..
make -C rpmbuilddir-%{_arch} VERBOSE=1 %{?_smp_mflags}
# << build pre



# >> build post
# << build post

%install
# >> install pre
#rm -rf %{buildroot}
DESTDIR=%{buildroot} make -C rpmbuilddir-%{_arch} install
mkdir -p %{_bindir}
# << install pre

# >> install post

desktop-file-install --delete-original       \
  --dir %{buildroot}%{_datadir}/applications             \
   %{buildroot}%{_datadir}/applications/*.desktop

## Jolla harbour rules:

# -- ship all shared unallowed libraries with the app
mkdir -p %{buildroot}%{_datadir}/%{name}/lib

# << install post

# -- check app rpath to find its libs
chrpath -l %{buildroot}%{_bindir}/%{name}
ls -al     %{buildroot}%{_bindir}/%{name}
sha1sum    %{buildroot}%{_bindir}/%{name}

%files
%defattr(-,root,root,-)
%{_bindir}
%{_datadir}/%{name}
%{_datadir}/%{name}/lib/
%{_datadir}/applications/%{name}.desktop
%{_datadir}/icons/hicolor/86x86/apps/%{name}.png
%{_datadir}/icons/hicolor/108x108/apps/%{name}.png
%{_datadir}/icons/hicolor/128x128/apps/%{name}.png
%{_datadir}/icons/hicolor/172x172/apps/%{name}.png
%{_datadir}/icons/hicolor/256x256/apps/%{name}.png
# >> files
# << files
