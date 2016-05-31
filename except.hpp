#ifndef EXCEPT_HEADER
#define EXCEPT_HEADER

enum except_ids {INVALID_TAG, EMPTY_OBJECT, WRONG_ENUM_VALUE, FILE_NOT_OPEN, ARRAY_OUT_OF_BOUNDS};

class Exception
{
    private:
        int id;
    public:
        Exception(int id): id(id) {}
        std::string what()
        {
            switch (id)
            {
                case(INVALID_TAG):
                    return "Tag read does not match the ASNobject's tag.";
                    break;
                case(EMPTY_OBJECT):
                    return "Tried to write/read an empty ASNobject.";
                    break;
                case(WRONG_ENUM_VALUE):
                    return "Tried to assign an undefined value to ASN_ENUMERATED.";
                    break;
                case(FILE_NOT_OPEN):
                    return "Couldn't open file for reading/writing.";
                    break;
                case(ARRAY_OUT_OF_BOUNDS):
                    return "Tried to access an element out of the array's bounds.";
                default:
                    return "Undefined exception thrown.";
            }
                
        }
};

#endif
