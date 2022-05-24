# Simple Filetype (sftp)
A very simple filetype with very little code required to decode it.
That's about it.
It has two primitive types:

## Variables
Example: `;MyVar = MyData`
The start of the variable is defined by a semicolon.
The end of the variables is defined by the newline character (0xA).

## Namespaces (Arrays)
Example:
`$MyNamespace (with some data that may be useful for decoding)
{
 Some data, some data, some data
}`
The start of a namespace is defined by the dollar sign '$'.
The data in the brackets is optional, but the brackets themselves aren't.
The last element must not have a comma behind it.
The opening and closing of a namespace is defined by curly brackets '{' and '}'.
