#include "profile.h"
#include <filesystem>
#include <fstream>

CProfile::CProfile(void)
{
    m_errcode = 0;
    std::error_code ec;
    m_path = std::filesystem::current_path(ec).string();
    if ( ec ) { m_path.clear(); return; }
    if ( m_path.empty() ) return;
    char back = m_path[m_path.size()-1];
    if ( back != '/' && back != '\\' ) {
        m_path += '/';
    }
}


CProfile::~CProfile(void)
{
}

std::string CProfile::lower( const std::string& s ) {
    std::string r = s;
    for ( size_t i = 0; i < r.size(); ++i ) {
        if ( r[i] >= 'A' && r[i] <= 'Z' ) r[i] = (char)(r[i] - 'A' + 'a');
    }
    return r;
}

std::string CProfile::trim( const std::string& s ) {
    size_t b = 0, e = s.size();
    while ( b < e && ( s[b] == ' ' || s[b] == '\t' || s[b] == '\r' || s[b] == '\n' ) ) ++b;
    while ( e > b && ( s[e-1] == ' ' || s[e-1] == '\t' || s[e-1] == '\r' || s[e-1] == '\n' ) ) --e;
    return s.substr(b, e - b);
}

std::vector<CProfile::Section> CProfile::load() const {
    std::vector<Section> sections;
    std::ifstream f( (m_path + m_filename).c_str(), std::ios::binary );
    if ( ! f ) return sections;
    Section* cur = 0;
    std::string line;
    while ( std::getline(f, line) ) {
        std::string t = trim(line);
        if ( t.empty() || t[0] == ';' || t[0] == '!' ) continue;
        if ( t[0] == '[' ) {
            size_t close = t.find(']');
            if ( close == std::string::npos ) continue;
            sections.push_back( Section() );
            sections.back().name = trim( t.substr(1, close - 1) );
            cur = &sections.back();
            continue;
        }
        if ( ! cur ) {
            sections.push_back( Section() );
            sections.back().name = "";
            cur = &sections.back();
        }
        size_t eq = t.find('=');
        if ( eq == std::string::npos ) continue;
        Entry e;
        e.key = trim( t.substr(0, eq) );
        e.value = trim( t.substr(eq + 1) );
        if ( e.value.size() >= 2 && e.value[0] == '"' && e.value[e.value.size()-1] == '"' ) {
            e.value = e.value.substr(1, e.value.size() - 2);
        }
        cur->entries.push_back( e );
    }
    return sections;
}

bool CProfile::save( const std::vector<Section>& sections ) const {
    std::ofstream f( (m_path + m_filename).c_str(), std::ios::binary | std::ios::trunc );
    if ( ! f ) return false;
    for ( size_t i = 0; i < sections.size(); ++i ) {
        f << "[" << sections[i].name << "]" << "\r\n";
        for ( size_t j = 0; j < sections[i].entries.size(); ++j ) {
            f << sections[i].entries[j].key << "=" << sections[i].entries[j].value << "\r\n";
        }
    }
    return f.good();
}

int CProfile::ReadString( std::string key, std::string& value ) {
    std::vector<Section> sections = load();
    std::string wantSec = lower( m_section );
    std::string wantKey = lower( key );
    for ( size_t i = 0; i < sections.size(); ++i ) {
        if ( lower( sections[i].name ) != wantSec ) continue;
        for ( size_t j = 0; j < sections[i].entries.size(); ++j ) {
            if ( lower( sections[i].entries[j].key ) == wantKey ) {
                value = sections[i].entries[j].value;
                return (int)value.size();
            }
        }
        break;
    }
    return 0;
}

int CProfile::WriteString( std::string key, std::string value ) {
    std::vector<Section> sections = load();
    std::string wantSec = lower( m_section );
    std::string wantKey = lower( key );
    int secIdx = -1;
    for ( size_t i = 0; i < sections.size(); ++i ) {
        if ( lower( sections[i].name ) == wantSec ) { secIdx = (int)i; break; }
    }
    if ( secIdx < 0 ) {
        sections.push_back( Section() );
        sections.back().name = m_section;
        secIdx = (int)sections.size() - 1;
    }
    Section& sec = sections[secIdx];
    for ( size_t j = 0; j < sec.entries.size(); ++j ) {
        if ( lower( sec.entries[j].key ) == wantKey ) {
            if ( value.empty() ) {
                sec.entries.erase( sec.entries.begin() + j );
            } else {
                sec.entries[j].value = value;
            }
            return save( sections ) ? 1 : 0;
        }
    }
    if ( value.empty() ) return 1;
    Entry e;
    e.key = key;
    e.value = value;
    sec.entries.push_back( e );
    return save( sections ) ? 1 : 0;
}
