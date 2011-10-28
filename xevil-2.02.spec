Summary: A fast-action, explicitly violent game for X
Name: xevil
Version: 2.02
Release: 1
Copyright: GPL
Group: X11/Games/Video
Source: ftp://ftp.xevil.com/stable/xevilsrc2.0r2.zip
Patch: xevil2.02-config.patch
Buildroot: /var/tmp/xevilroot
Summary(de): Ein schnelles, extrem gewalttätiges Actionspiel für X
Summary(fr): Un jeu d'action rapide et très violent sous X
Summary(tr): Hýzlý ve þiddet yüklü bir X oyunu

%changelog
* Mon Jan 25 1999 Steve Hardt <hardts@best.com>

- updated for XEvil 2.0
- more accurate description, need new translations of description

* Thu May 07 1998 Prospector System <bugs@redhat.com>

- translations modified for de, fr, tr


* Thu Oct 09 1997 Erik Troan <ewt@redhat.com>

- updated to work on non Intel platforms (silly Makefile hack)
- uses a buildroot and %attr tags

* Fri Aug 22 1997 Erik Troan <ewt@redhat.com>

- built against glibc

%description 
Death, blood, drugs, fire, ninjas, and chainsaws.  XEvil is a
side-view, fast-action, single or network-multiplayer, anti-social,
kill-everything game for UNIX and Windows.  You have died and gone to
Hell.  XEvil is the contest that determines your fate in Hell for all
eternity.


%prep
# Don't use %setup because the source is not a compressed tar file.
rm -rf $RPM_BUILD_DIR/xevil-2.02
mkdir $RPM_BUILD_DIR/xevil-2.02
cd $RPM_BUILD_DIR/xevil-2.02
# Text and binary files, auto-convert CRLF
/usr/bin/unzip -a $RPM_SOURCE_DIR/xevilsrc2.0r2.zip -x win32/res/\*.bmp -x win32/\*.dsp
# Binary files, don't convert CRLF
/usr/bin/unzip -b $RPM_SOURCE_DIR/xevilsrc2.0r2.zip win32/res/\*.bmp win32/\*.dsp
chown -R root.root .
chmod -R a+rX,g-w,o-w .
%patch -p1

%build
cd $RPM_BUILD_DIR/xevil-2.02
make

%install
rm -rf $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT/usr/X11R6/bin
mkdir -p $RPM_BUILD_ROOT/etc/X11/wmconfig

cd $RPM_BUILD_DIR/xevil-2.02
install -m 755 x11/REDHAT_LINUX/xevil $RPM_BUILD_ROOT/usr/X11R6/bin

cat > $RPM_BUILD_ROOT/etc/X11/wmconfig/xevil <<EOF
xevil name "xevil"
xevil description "Side-View Action Game"
xevil group Games/Video
xevil exec "xevil &"
EOF

%files
%doc xevil-2.02/instructions/instructions.html 
%doc xevil-2.02/instructions/keysdlg.gif 
%doc xevil-2.02/instructions/optionsdlg.gif 
%doc xevil-2.02/instructions/UNIXmenu.gif 
%doc xevil-2.02/instructions/UNIXSetControls.gif 
%doc xevil-2.02/instructions/UNIXShowControls.gif
%attr(-,root,root) /usr/X11R6/bin/xevil
%config %attr(-,root,root) /etc/X11/wmconfig/xevil


