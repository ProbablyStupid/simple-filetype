#define SFTP_STANDARD_IMPLEMENTATION
#include "..\simple filetype\sftp.h"

#include "..\sftp-standard\sftp-standard.h"

int main()
{
	sftp_contents myContents;

	sftp_file myFile("example.sftp");
	myContents = myFile.get_results();

	std::cout << myContents.namespaces[0].name << std::endl;

	sftp_standard myStandard = read_standard("example_standard.txt");
	
	std::cout << myStandard.name << myStandard.purpose << myStandard.version_string
		<< myStandard.required_fields[0] << myStandard.required_namespaces[0].name
		<< std::endl;

	return 0;
}