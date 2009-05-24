#
# Spec file for samefile-3.0
#
Name: samefile
Version: 3.0
Release: 0
URL:
Source: http://www.akruijff.dds.nl/samesame/%{name}-%{version}.tar.gz
License: BSD (2 Clauses), Copyright (c) 2009 Alex de kruijff
Group: Applications/File
Summary: Find identical files and optionally replaces them with links.
BuildRoot:    %{_tmppath}/%{name}-%{version}-build

%description
These applications find identical / duplicate files and hard- or
symlink them to save space. This port was inspired by the samefile v2.12 .
It has a own version of samefile that is noticeable faster.

This port contains the applications: samefile, samelink and samearchive.

- Samefile reads a list of filenames from the stdin and outputs the
  identical files.

- Samelink reads the samefile output and creates relinks the files to
  save space.

- Samearchive reads a list of filenames from the stdin and reads a list
  of directories passed as paramenters and outputs the identical files,
  if the relative paths match.

- Samearchive-lite does the same job as samearchive, but it does 80% of
  the work in 50% of the time and with 20% of the resources.

Typical usage would be:

        find / | samefile -i | samelink

This would search for identical files and clean up wasted disk space by
linking them together. Add the option -n after samelink for a dry-run.

%prep

%setup

%build
%configure
make

%install
[ "$RPM_BUILD_ROOT" != "/" ] && [ -d $RPM_BUILD_ROOT ] && rm -rf $RPM_BUILD_ROOT;
%makeinstall

%clean
[ "$RPM_BUILD_ROOT" != "/" ] && [ -d $RPM_BUILD_ROOT ] && rm -rf $RPM_BUILD_ROOT;

%files
%doc INSTALL README
%doc %{_mandir}/man1/%{name}.1*
%{_prefix}/bin/%{name}

%changelog

- Initial spec file created for version 3.01
