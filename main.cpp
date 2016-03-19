#include <iostream>
#include <map>
#include <vector>
#include <algorithm>

#include "mounts.h"

using namespace std;

#define assert(expr) my_assert(expr, #expr, __LINE__)

bool my_assert(bool expr, string const & name, int line) {
    std::cout << (expr ? "[PASS] " : "[FAIL] ") << name << " in line " << line << std::endl;
    return expr;
}

int main()
{
    Mounts m("C:\\Users\\admin\\documents");

    m.mount("/work", "W:\\Dokumente\\");
    m.mount("/nas", "N:\\documents");
    m.mount("/work/kunde1","W:\\kunde1");
    m.mount("/work/kunde2", "W:\\kunde2");
    m.mount("/work/kunde1/shared", "W:\\shared");
    m.mount("/work/kunde2/shared", "W:\\shared");

    //assert(m.absolutePath("/") == "C:\\Users\\admin\\documents");

//    auto v = m.allMountPathes("C:\\Users\\admin\\documents");
//    assert(1 == v.size());
//    assert("/" == v.front());

//    v = m.allMountPathes("C:\\");
//    assert(v.size() == 0);

    {
        auto result = m.absolutePath("/work/kunde1/Vertrag.doc");
        cout << result << endl;
        assert(result == "W:\\kunde1/Vertrag.doc");
    }

    {
        auto result = m.allMountPathes("W:\\shared\\Styleguide.md");
        if(assert(result.size()==2))  {
            assert(find(result.begin(),result.end(),("/work/kunde1/shared\\Styleguide.md"))!=result.end());
            assert(find(result.begin(),result.end(),("/work/kunde2/shared\\Styleguide.md"))!=result.end());
        }
    }


    {
        auto result = m.getMountPointsBelow("/work/kunde1");
        if(assert(result.size()==2)) {
            assert(find(result.begin(),result.end(),pair<string,string>("/work/kunde1", "W:\\kunde1"))!=result.end());
            assert(find(result.begin(),result.end(),pair<string,string>("/work/kunde1/shared", "W:\\shared"))!=result.end());
        }
    }

    {
        auto m2 = m;
        m2.unmountAbsolute("W:\\shared");
        auto result = m2.allMountPathes("W:\\shared");
        assert(result.size() == 0);
    }

    {
        auto m2 = m;
        m2.unmountBelow("/work/kunde1");
        auto result = m2.allMountPathes("W:\\kunde1");
        assert(result.size() == 0);
    }

    return 0;
}


