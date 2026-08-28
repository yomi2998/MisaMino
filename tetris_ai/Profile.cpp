#include "Profile.h"


CProfile::CProfile(void)
{
    char buff[1024];
    m_errcode = 0;
    buff[0] = 0;
    ::GetCurrentDirectoryA(1024, buff);
    m_path = buff;
    if ( m_path.empty() ) return;
    char back = m_path[m_path.size()-1];
    if ( back != '\\' && back != '/' ) {
        m_path += '\\';
    }
}


CProfile::~CProfile(void)
{
}
