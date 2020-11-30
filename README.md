# encrypt-decrypt-files-using-S-DES
Program to encrypt and decrypt files using S-DES (Simplified DES) in the Cipher Block Chaining (CBC) mode

To compile type command 'make' at the terminal

to encrypt file
./mycypher <10-bit key> <8-bit initial vector> <source_file> <result_file>

to decrypt file
./mycyper -d <10-bit key> <8-bit initial vector> <result_file (from step above)> <new_result file>

to delete files (except code and source files)
type command 'make clean at the command prompt
