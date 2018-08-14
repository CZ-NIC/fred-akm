.PHONY: srpm

.SHELLFLAGS := -xc

spec_dir := `rpm --eval "%{_specdir}"`
source_dir := `rpm --eval "%{_sourcedir}"`
srcrpm_dir := `rpm --eval "%{_srcrpmdir}"`

our_version := `git describe --tags | cut -d'-' -f1`
our_release := `git describe --tags | cut -s -d'-' -f2- | tr '-' '_'`

project_name := fred-akm
spec_file := $(project_name).spec

srpm:
	rpm -q git > /dev/null 2>&1 || dnf -y install git
	cd 3rd_party/sqlite3pp && \
	git archive --prefix=$(project_name)-$(our_version)/3rd_party/sqlite3pp/ --format=tar HEAD -o ../../"$(source_dir)/.sqlite3pp.tar"
	mkdir -p "$(source_dir)" && \
	mkdir -p "$(spec_dir)" && \
	git archive --prefix=$(project_name)-$(our_version)/ --format=tar HEAD -o "$(source_dir)/$(project_name)-$(our_version).tar" && \
	tar --concatenate --file="$(source_dir)/$(project_name)-$(our_version).tar" .sqlite3pp.tar && \
	gzip "$(source_dir)/$(project_name)-$(our_version).tar"
	echo "%define our_version $(our_version)" > $(spec_dir)/$(spec_file) && \
	echo "%define our_release $(our_release)" >> $(spec_dir)/$(spec_file) && \
	cat .copr/$(spec_file) >> $(spec_dir)/$(spec_file) && \
	rpmbuild -bs $(spec_dir)/$(spec_file) && \
	mv $(srcrpm_dir)/$(project_name)*.src.rpm $(outdir)
