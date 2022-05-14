#pragma once

#include <iostream>
#include <fstream>
#include <filesystem>
#include <vector>

struct sftp_variable
{
	std::string name;
	std::string data;
};

struct sftp_namespace
{
	std::string name;
	std::string annotation;
	std::vector<std::string> data;
};

typedef struct sftp_contents
{
	std::vector<sftp_variable> variables;
	std::vector<sftp_namespace> namespaces;
} sftp_contents;

class sftp_compiler
{
private:
	std::vector<sftp_variable> m_Variables;
	std::vector<sftp_namespace> m_Namespaces;
public:
	
	// for removing whitespaces at the beginnning and end of a string i.e.:
	// "		this statement  has no meaning  " -> "this statement  has no meaning"
	inline std::string isolate_string_from_sequence(std::string sequence)
	{
		uint64_t first = 0, last = 0;
		for (uint64_t i = 0; i < sequence.size(); i++)
		{
			if (sequence[i] != ' ')
			{
				first = i;
				break;
			}
		}
		for (uint64_t i = (sequence.size() - 1); i >= 0; i--)
		{
			if (sequence[i] != ' ')
			{
				last = i;
				break;
			}
		}
		return sequence.substr(first, (last - first + 1));
	}

	void compile_file(std::string contents);

	std::vector<sftp_namespace> get_namespaces();
	std::vector<sftp_variable> get_variables();

};

class sftp_file
{
private:
	std::string m_Contents;
	sftp_contents m_Compiled;

public:
	sftp_file(std::string filepath);
	sftp_contents get_results();
};

class sftp_writer
{
private:

	std::vector<sftp_namespace> m_Namespaces;
	std::vector<sftp_variable> m_Variables;
	bool m_Written = false;

	std::string m_Filepath;

public:

	sftp_writer(std::string filepath);

	inline std::string namespace_to_string(sftp_namespace& m)
	{
		std::stringstream string;
		string << '$' << m.name << ' (' << m.annotation << ')\n{\n';
		
		for (uint64_t i = 0; i < (m.data.size() - 1); i++)
			string << m.data[i] << ', ';

		string << m.data[m.data.size() - 1];
		string << '}';

		return string.str();
	}

	inline std::string variable_to_string(sftp_variable& v)
	{
		std::stringstream string;
		string << ';' << v.name << ' = ' << v.data << '\n';
		return string.str();
	}

	void add_namespace(sftp_namespace& m);
	void add_variable(sftp_variable& v);
	
	void add_namespaces(sftp_namespace* m, size_t size);
	void add_variables(sftp_variable* v, size_t size);

	void write();
};

#ifdef SFTP_STANDARD_IMPLEMENTATION

sftp_writer::sftp_writer(std::string filepath)
 : m_Filepath(filepath) {}

void sftp_writer::add_namespace(sftp_namespace& m)
{
	m_Namespaces.push_back(m);
}

void sftp_writer::add_variable(sftp_variable& v)
{
	m_Variables.push_back(v);
}

void sftp_writer::add_namespaces(sftp_namespace* m, size_t size)
{
	m_Namespaces.insert(m_Namespaces.end(), m, m + size);
}

void sftp_writer::add_variables(sftp_variable* v, size_t size)
{
	m_Variables.insert(m_Variables.end(), v, v + size);
}

void sftp_writer::write()
{
	std::stringstream string;
	for (sftp_variable i : m_Variables)
		string << variable_to_string(i);
	for (sftp_namespace i : m_Namespaces)
		string << namespace_to_string(i);
	
	std::string final_string = string.str();

	std::ofstream myFile(m_Filepath);
	myFile.write(final_string.c_str(), final_string.size());
	myFile.close();
}

std::vector<sftp_namespace> sftp_compiler::get_namespaces()
{
	return m_Namespaces;
}

std::vector<sftp_variable> sftp_compiler::get_variables()
{
	return m_Variables;
}

void sftp_compiler::compile_file(std::string contents)
{
	// variables first

	std::vector<uint64_t> variable_indicies;
	std::vector<uint64_t> variable_mid_indicies;
	std::vector<uint64_t> variable_end_indicies;

	// find the variables

	bool invalid_char = false;
	for (uint64_t i = 0; i < contents.size(); i++)
	{
		/*
		if (i > 0)
			if (contents[i] == ';' && contents[i-1] != '\\')
				variable_indicies.push_back(i);
		else
			if (contents[i] == ';')
				variable_indicies.push_back(i);

		if (i > 0)
			if (contents[i] == '=' && contents[i - 1] != '\\')
				variable_mid_indicies.push_back(i);
			else
				if (contents[i] == '=')
					variable_mid_indicies.push_back(i);

		if (i > 0)
			if (contents[i] == '\n' && contents[i - 1] != '\\')
				variable_end_indicies.push_back(i);
			else
				if (contents[i] == '\n')
					variable_end_indicies.push_back(i);
		*/

		if (!invalid_char)
			switch (contents[i])
			{
			case ';':
				variable_indicies.push_back(i);
				break;
			case '=':
				variable_mid_indicies.push_back(i);
				break;
			case '\n':
				variable_end_indicies.push_back(i);
				break;
			case '\\':
				invalid_char = true;
				break;
			}
		else
			invalid_char = false;
	}

	// evaluate them
	for (uint64_t i = 0; i < variable_indicies.size(); i++)
	{
		sftp_variable myVar;
		myVar.name = isolate_string_from_sequence(contents.substr(variable_indicies[i], variable_mid_indicies[i]));
		myVar.data = isolate_string_from_sequence(contents.substr(variable_mid_indicies[i], variable_end_indicies[i]));
	}

	//namespaces

	std::vector<uint64_t> namespace_indicies;
	std::vector<uint64_t> namespace_annotation_start;
	std::vector<uint64_t> namespace_annotation_end;
	std::vector<uint64_t> namespace_space;
	std::vector<uint64_t> namespace_end;
	std::vector<std::vector<uint64_t>> namespace_commas;

	bool valid_char = true;

	for (uint64_t i = 0; i < contents.size(); i++)
	{
		if (valid_char)
			switch (contents[i])
			{
			case '\\':
				valid_char = false;
				break;
			case '$':
				namespace_indicies.push_back(i);
				break;
			case '(':
				namespace_annotation_start.push_back(i);
				break;
			case ')':
				namespace_annotation_end.push_back(i);
				break;
			case '{':
				namespace_commas.push_back(std::vector<uint64_t>());
				namespace_space.push_back(i);
				break;
			case '}':
				namespace_end.push_back(i);
				break;
			case ',':
				namespace_commas.back().push_back(i);
				break;
			}
		else
			valid_char = true;
	}

	for (uint64_t i = 0; i < namespace_indicies.size(); i++)
	{
		sftp_namespace x;
		const auto name_substr_length = (namespace_annotation_start[i] - (namespace_indicies[i] + 1));
		const auto name_substr = contents.substr((namespace_indicies[i] + 1), name_substr_length);
		const auto name = isolate_string_from_sequence(name_substr);
		x.name = name;
		const auto annotation = isolate_string_from_sequence(contents.substr(namespace_annotation_start[i], (namespace_annotation_end[i] - namespace_annotation_start[i])));
		x.annotation = annotation;
		if (namespace_commas[i].size() > 0)
		{
			const auto first_element = isolate_string_from_sequence(contents.substr(namespace_space[i], (namespace_commas[i][0] - namespace_space[i])));
			x.data.push_back(first_element);
			if (namespace_commas[i].size() > 2) {
				for (uint64_t j = 1; j < (namespace_commas[i].size() - 2); j++)
					x.data.push_back(isolate_string_from_sequence(contents.substr(namespace_commas[i][j], (namespace_commas[i][j + 1] - namespace_commas[i][j]))));
				goto SKIP_IF;
			}
			if (namespace_commas[i].size() > 1)
			{
			SKIP_IF:
				x.data.push_back(isolate_string_from_sequence(contents.substr(namespace_commas[i][namespace_commas[i].size() - 1],
					(namespace_end[i] - namespace_commas[i][namespace_commas[i].size() - 1]))));
			}
		}
		m_Namespaces.push_back(x);
	}
}

sftp_file::sftp_file(std::string filepath)
{
	std::ifstream myStream(filepath);
	std::filesystem::path p = filepath;
	std::uintmax_t size = std::filesystem::file_size(p);
	std::vector<char> contents(size);
	myStream.read(&contents[0], size);
	m_Contents = contents.data();

	sftp_compiler myCompiler;
	myCompiler.compile_file(m_Contents);

	m_Compiled.variables = myCompiler.get_variables();
	m_Compiled.namespaces = myCompiler.get_namespaces();
}

sftp_contents sftp_file::get_results()
{
	return m_Compiled;
}

#endif