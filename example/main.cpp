#define SFTP_STANDARD_IMPLEMENTATION
#include "..\simple filetype\sftp.h"

int main()
{
	sftp_contents myContents;

	sftp_file myFile("example.sftp");
	myContents = myFile.get_results();

	std::cout << myContents.namespaces[0].name << std::endl;

	return 0;
}