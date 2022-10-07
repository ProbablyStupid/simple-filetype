//pure headerfile
#pragma once

#include <iostream>
#include <vector>

#include "../simple filetype/sftp.h"

typedef struct sftp_namespace_setting
{
	std::string name;
	bool annotation_required;
	uint64_t required_entries;
} sftp_namespace_setting;

typedef struct sftp_standard
{
	std::string name;
	std::string version_string;
	unsigned version_major;
	unsigned version_minor;
	std::string purpose;

	std::vector<std::string> required_fields;
	std::vector<sftp_namespace_setting> required_namespaces;
} sftp_standard;

typedef struct sftp_namespace_review
{
	bool annotation_missing;
	uint64_t entries_missing;
} sftp_namespace_review;

typedef struct sftp_standard_review
{
	std::vector<std::string> missing_fields;
	std::vector<std::string> missing_namespaces;
	std::vector<sftp_namespace_review> incorrect_namespaces;
} sftp_standard_review;

sftp_standard read_standard(std::string filepath);

sftp_standard_review check_file_for_standard(sftp_contents* file,
	sftp_standard* standard);