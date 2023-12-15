package main

/*
#  include <stdlib.h>
typedef struct FileInfo{
    int Size;
    char *Name;
}FileInfo;

typedef struct Result{
    FileInfo **files;
}Result;

int GetResult(void **presult,FileInfo **files) {
    Result *result = (Result *) malloc(sizeof(Result));
    result->files=files;
    *presult = result;
    int ptr = (int) result;
    return ptr;
}
*/
import "C"
import "unsafe"

// https://www.jb51.cc/faq/2889194.html
// 分析
// arr[0]包含&ai，它是Go分配的C.struct_FileInfo的指针。
// arr也由Go管理，因此&arr[0]是“指向Go指针的Go指针”。
// 可能的解决方案：调用C.malloc分配足够的字节来存储C.struct_FileInfo。这样，您将拥有指向C指针的Go指针（＆arr [0]）
func main() {
	var arr []*C.FileInfo
	ai := C.struct_FileInfo{
		Size: C.int(1234),
		Name: C.CString("some name"),
	}

	arr = append(arr, &ai)

	var presult unsafe.Pointer

	ptr := C.GetResult(&presult, &arr[0])
	println("\nResult struct pointer: %v", ptr)
}
