Name:           ${name}
Version:        ${version}
Release:        1%{?dist}
Summary:        ${summary}
License:        ${license}
URL:            ${url}
Source0:        ${source_filename}

%description
${summary}

%prep
%autosetup -n ${source_dir}

%build
%cmake
%cmake_build

%install
%cmake_install

%files
%{_bindir}/*
