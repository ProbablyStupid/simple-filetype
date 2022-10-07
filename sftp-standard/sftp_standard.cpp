#include "sftp-standard.h"

#include <fstream>

std::string read_file(std::string filepath)
{
	std::filesystem::path path = filepath;
	const auto file_size = std::filesystem::file_size(path);
	auto* contents = static_cast<char*>(malloc(file_size * sizeof(char)));

	std::ifstream myStream(filepath);
	myStream.read(contents, file_size);

	std::string myString;
	
	if (contents != nullptr)
		myString = contents;
	else
		throw std::runtime_error("Error reading file: file was NULL");

	free(contents);

	return myString;
}

sftp_standard read_standard(std::string filepath)
{
	std::string file = read_file(filepath);
	
	// file headers
	
	size_t name_location = file.find("|NAME|");
	size_t name_line_end = 0;
	for (size_t i = name_location; i < (name_location + 0xffff); i++)
	{
		if (file[i] == '\n')
		{
			name_line_end = i;
			goto clear;
		}
	}
	std::cout << "No standard-name found" << std::endl;
	throw std::runtime_error("No standard-name found");
clear:

	std::string name = file.substr(name_location + 6, (name_line_end - (name_location + 6)));

	size_t version_location = file.find("|VERSION|");
	size_t version_line_end = 0;
	for (size_t i = version_location; i < (version_location + 0xffff); i++)
	{
		if (file[i] == '\n')
		{
			version_line_end = i;
			goto clear2;
		}
	}
	std::cout << "No standard-version found" << std::endl;
	throw std::runtime_error("No standard-version found");
	
clear2:
	std::string version = file.substr(version_location + 9, version_line_end - (version_location + 9));

	size_t purpose_location = file.find("|PURPOSE|");
	size_t purpose_line_end;
	for (size_t i = purpose_location; i < (purpose_location + 0xffff); i++)
	{
		if (file[i] == '\n')
		{
			purpose_line_end = i;
			goto clear3;
		}
	}
	std::cout << "No standard-purpose found" << std::endl;
	throw std::runtime_error("No standard-purpose found");

clear3:
	std::string purpose = file.substr(purpose_location + 9, purpose_line_end - (purpose_location + 9));

	// standard parsing

	std::vector<size_t> required_field_locations;
	std::vector<size_t> required_space_locations;

	for (size_t i = 0; i < file.size(); i++)
	{
		switch (file[i])
		{
		case '/':
			required_field_locations.push_back(i);
			break;
		case '\\':
			required_space_locations.push_back(i);
			break;
		}
	}
	
	std::vector<std::string> required_fields;
	std::vector<std::string> required_spaces;

	// fields
	for (size_t i = 0; i < required_field_locations.size(); i++)
	{
		size_t line_end;
		for (size_t j = required_field_locations[i]; j < (required_field_locations[i] + 0xffff); j++)
		{
			if (file[j] == '\n')
			{
				line_end = j;
				goto clear_fields;
			}
		}
		throw std::runtime_error("Line is too long");

	clear_fields:
		required_fields.push_back(file.substr(required_field_locations[i] + 1, line_end - (required_field_locations[i] + 1)));
	}

	// spaces
	for (size_t i = 0; i < required_space_locations.size(); i++)
	{
		size_t line_end;
		for (size_t j = required_space_locations[i]; j < (required_space_locations[i] + 0xffff); j++)
		{
			if (file[j] == '\n')
			{
				line_end = j;
				goto clear_spaces;
			}
		}
		throw std::runtime_error("Line is too long");

	clear_spaces:
		required_spaces.push_back(file.substr(required_space_locations[i], line_end - (required_space_locations[i] + 1)));
	}

	// TOOD: Add compilation of individual required spaces (r){x}

	std::vector<sftp_namespace_setting> required_namespaces;
	
	for (std::string i : required_spaces)
	{
		uint16_t brackets_open = 0, brackets_close = 0, curly_open = 0, curly_close = 0;
		for (uint16_t j = 0; j < i.size(); j++)
		{
			switch (i[j])
			{
			case '(':
				brackets_open = j;
				break;
			case ')':
				brackets_close = j;
				break;
			case '{':
				curly_open = j;
				break;
			case '}':
				curly_close = j;
				break;
			}
		}

		sftp_namespace_setting myNamespaceSetting;
		myNamespaceSetting.name = i.substr(1, brackets_open - 1);
		myNamespaceSetting.annotation_required = false;

		for (uint16_t j = curly_open; j <= curly_close; j++)
		{
			if (i[j] == 'r')
			{
				myNamespaceSetting.annotation_required = true;
				break;
			}
		}


		if ((curly_close - curly_open) > 1)
		{
			short* myArray = new short[curly_close - curly_open];
			for (uint16_t j = curly_open; j <= curly_close; j++)
				myArray[curly_close - j] = (char)((short)i[j] - 30);

			uint16_t myNumber = 0;
			uint16_t current_multiplier = 1;
			for (uint16_t j = curly_close; j >= curly_open; j--)
			{
				myNumber += myArray[curly_close - j] * current_multiplier;
				current_multiplier *= 10;
			}

			myNamespaceSetting.required_entries = myNumber;
		}

		required_namespaces.push_back(myNamespaceSetting);
	}
	
	sftp_standard myStandard;
	myStandard.name = name;
	myStandard.purpose = purpose;
	// TODO: Add parsing for version major / version minor
	myStandard.version_string = version;
	myStandard.required_fields = required_fields;
	myStandard.required_namespaces = required_namespaces;

	return myStandard;
}

sftp_standard_review check_file_for_standard(sftp_contents* file, sftp_standard* standard)
{
	// fields

	bool has_all_fields = true;
	std::vector<std::string> missing_fields;

	for (auto i : standard->required_fields)
	{
		if (!file->variables.contains(i))
		{
			has_all_fields = false;
			missing_fields.push_back(i);
		}
	}

	// spaces

	bool has_all_spaces = true;
	bool namespace_errors = false;
	std::vector<std::string> missing_spaces;
	std::vector<std::string> missing_annotations;
	std::vector<std::string> small_spaces;

	for (auto i : standard->required_namespaces)
	{
		if (!file->namespaces.contains(i.name))
		{
			has_all_spaces = false;
			namespace_errors = true;
			missing_spaces.push_back(i.name);
		}
		if (file->namespaces[i.name].annotation.empty())
		{
			namespace_errors = true;
			missing_annotations.push_back(i.name);
		}
		if (file->namespaces[i.name].data.size() != i.required_entries)
		{
			namespace_errors = true;
			small_spaces.push_back(i.name);
		}
	}

	// review

	sftp_standard_review myReview
	{
		missing_fields,
		missing_spaces,
		missing_annotations,
		small_spaces,
		has_all_fields,
		has_all_spaces,
		namespace_errors
	};
	return myReview;
}