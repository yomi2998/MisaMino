#pragma once
#include <string>
#include <cstdio>
#include <cstdlib>
#include <vector>

class CProfile
{
public:
    CProfile(void) ;
    virtual ~CProfile(void);
    void SetFile( std::string name ) {
        m_filename = name;
    }
    void SetSection( std::string section ) {
        m_section = section;
    }
    int WriteString( std::string key, std::string value );
    int WriteInteger( std::string key, int value ) {
        char buff[1024];
        snprintf( buff, sizeof(buff), "%d", value );
        return WriteString( key, buff );
    }
    int ReadString( std::string key, std::string& value );
    bool IsInteger( std::string key ) {
        ReadInteger( key );
        return m_errcode == 0;
    }
    int ReadInteger( std::string key ) {
        std::string s;
        if ( ReadString( key, s ) > 0 ) {
            int ret;
            if ( sscanf( s.c_str(), "%d", &ret ) > 0 ) {
                m_errcode = 0;
                return ret;
            }
            m_errcode = 1;
        }
        m_errcode = 2;
        return 0;
    }
private:
    struct Entry {
        std::string key;
        std::string value;
    };
    struct Section {
        std::string name;
        std::vector<Entry> entries;
    };
    std::vector<Section> load() const;
    bool save( const std::vector<Section>& sections ) const;
    static std::string lower( const std::string& s );
    static std::string trim( const std::string& s );
    int m_errcode;
    std::string m_path;
    std::string m_filename;
    std::string m_section;
};
