#pragma once

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4244 4245 4267)
#endif
#include "external/miniaudio.h"
#ifdef _MSC_VER
#pragma warning(pop)
#endif
#include "random.h"
#include <deque>
#include <string>
#include <cstdio>

class GameSound {
protected:
    GameSound() {
        mVolume = 0.5f;
        mMusicRatio = 0.6f;
        mMusicMuted = false;
        mBgmVolume = mVolume * mMusicRatio;
        mReady = false;
        ma_engine_config cfg = ma_engine_config_init();
        mEngine = new ma_engine;
        if (ma_engine_init(&cfg, mEngine) == MA_SUCCESS) {
            ma_engine_start(mEngine);
            mReady = true;
        } else {
            delete mEngine;
            mEngine = NULL;
        }
    }
    ~GameSound() {
        unloadAll();
        if (mEngine) {
            ma_engine_uninit(mEngine);
            delete mEngine;
            mEngine = NULL;
        }
    }
    void unloadAll() {
        sound* all[] = {
            &mSFX_move, &mSFX_hold, &mSFX_rotate, &mSFX_softdrop, &mSFX_harddrop, &mSFX_lockdown,
            &mSFX_lineattack, &mSFX_b2b_tetris, &mSFX_ko, &mSFX_gameover, &mSFX_pc, &mbgm
        };
        for (int i = 0; i < 12; ++i) all[i]->unload();
        for ( int i = 0; i < 4; ++i ) mSFX_clears[i].unload();
        for ( int i = 0; i < 20; ++i ) mSFX_combo[i].unload();
        for ( int i = 0; i < 4; ++i ) mSFX_tspin[i].unload();
        for ( int i = 0; i < 4; ++i ) mSFX_b2b_tspin[i].unload();
    }
public:
    class sound {
    public:
        sound( ) {
            for ( int i = 0; i < 16; ++i)
                m[i] = NULL;
            mOpen = false;
            mIndex = 0;
            mMax = 1;
        }
        ~sound() {
            unload();
        }
        void unload() {
            for ( int i = 0; i < 16; ++i) {
                if ( m[i] ) {
                    ma_sound_uninit(m[i]);
                    delete m[i];
                    m[i] = NULL;
                }
            }
            mOpen = false;
        }
        int open( float * volume, const char* path, int nMax = 2, int loop = 0 ) {
            mVolume = volume;
            for ( int i = 0; i < 16; ++i) {
                if ( m[i] ) {
                    ma_sound_uninit(m[i]);
                    delete m[i];
                    m[i] = NULL;
                }
            }
            int loaded = 0;
            if ( GameSound::ins().mEngine ) {
                for ( int i = 0; i < nMax && i < 16; ++i) {
                    ma_sound* s = new ma_sound;
                    if ( ma_sound_init_from_file(GameSound::ins().mEngine, path, 0, NULL, NULL, s) == MA_SUCCESS ) {
                        if ( loop ) ma_sound_set_looping(s, MA_TRUE);
                        m[i] = s;
                        ++loaded;
                    } else {
                        delete s;
                        break;
                    }
                }
            }
            mMax = loaded > 0 ? loaded : 1;
            mOpen = loaded > 0;
            return loaded;
        }
        bool isOpen() {
            return mOpen;
        }
        int play( int lr = 0) {
            if ( ! mOpen ) return 0;
            mIndex = (mIndex+1) % mMax;
            if ( m[mIndex] ) {
                if ( lr == 1 ) {
                    ma_sound_set_pan(m[mIndex], -0.7);
                } else if ( lr == 2 ) {
                    ma_sound_set_pan(m[mIndex], 0.7);
                } else {
                    ma_sound_set_pan(m[mIndex], 0);
                }
                if ( mVolume ) ma_sound_set_volume(m[mIndex], *mVolume);
                ma_sound_stop(m[mIndex]);
                return ma_sound_start(m[mIndex]) == MA_SUCCESS ? 1 : 0;
            }
            return 0;
        }
        int stop() {
            if ( m[mIndex] ) {
                return ma_sound_stop(m[mIndex]) == MA_SUCCESS ? 1 : 0;
            }
            return 0;
        }
        void setVolume( float volume ) {
            if ( m[mIndex] ) {
                ma_sound_set_volume( m[mIndex], volume );
            }
        }
    private:
        ma_sound* m[16];
        int mIndex;
        int mMax;
        bool mOpen;
        float * mVolume;
    };
    static GameSound& ins() {
        static GameSound gamesound;
        return gamesound;
    }
    void loadSFX () {
        std::string basePath = "sound/sfx/default/";
        mSFX_ko.open(&mVolume, (basePath + "sfx_ko.wav").c_str());
        mSFX_gameover.open(&mVolume, (basePath + "sfx_gameover.wav").c_str());
        mSFX_pc.open(&mVolume, (basePath + "sfx_perfectclear.wav").c_str());
        mSFX_move.open(&mVolume, (basePath + "sfx_move.wav").c_str(), 4);
        mSFX_hold.open(&mVolume, (basePath + "sfx_hold.wav").c_str());
        mSFX_rotate.open(&mVolume, (basePath + "sfx_rotate.wav").c_str());
        mSFX_softdrop.open(&mVolume, (basePath + "sfx_softdrop.wav").c_str(), 8);
        mSFX_harddrop.open(&mVolume, (basePath + "sfx_harddrop.wav").c_str());
        mSFX_lockdown.open(&mVolume, (basePath + "sfx_lockdown.wav").c_str());
        mSFX_lineattack.open(&mVolume, (basePath + "sfx_lineattack.wav").c_str());
        mSFX_clears[0].open(&mVolume, (basePath + "sfx_single.wav").c_str());
        mSFX_clears[1].open(&mVolume, (basePath + "sfx_double.wav").c_str());
        mSFX_clears[2].open(&mVolume, (basePath + "sfx_triple.wav").c_str());
        mSFX_clears[3].open(&mVolume, (basePath + "sfx_tetris.wav").c_str());
        mSFX_b2b_tetris.open(&mVolume, (basePath + "sfx_b2b_tetris.wav").c_str());
        for ( int i = 0; i < 20; ++i) {
            char name[16];
            snprintf( name, sizeof(name), "sfx_combo%d.wav", i + 1);
            mSFX_combo[i].open(&mVolume, (basePath + name).c_str());
        }
        mSFX_tspin[0].open(&mVolume, (basePath + "sfx_tspin_mini.wav").c_str());
        mSFX_tspin[1].open(&mVolume, (basePath + "sfx_tspin_single.wav").c_str());
        mSFX_tspin[2].open(&mVolume, (basePath + "sfx_tspin_double.wav").c_str());
        mSFX_tspin[3].open(&mVolume, (basePath + "sfx_tspin_triple.wav").c_str());
        mSFX_b2b_tspin[0].open(&mVolume, (basePath + "sfx_b2b_tspin_mini.wav").c_str());
        mSFX_b2b_tspin[1].open(&mVolume, (basePath + "sfx_b2b_tspin_single.wav").c_str());
        mSFX_b2b_tspin[2].open(&mVolume, (basePath + "sfx_b2b_tspin_double.wav").c_str());
        mSFX_b2b_tspin[3].open(&mVolume, (basePath + "sfx_b2b_tspin_triple.wav").c_str());
    }
    void loadBGM_wait( AI::Random& rnd ) {
        std::string basePath = "sound/music/default/";
        int n = rnd.randint(2);
        if ( n == 0 ) {
            mbgm.open(&mBgmVolume, (basePath + "waiting1.ogg").c_str(), 1, 1);
        } else {
            mbgm.open(&mBgmVolume, (basePath + "waiting2.ogg").c_str(), 1, 1);
        }
        mbgm.play();
    }
    void loadBGM( AI::Random& rnd ) {
        std::string basePath = "sound/music/default/";
        int n = rnd.randint(3);
        if ( n == 0 ) {
            mbgm.open(&mBgmVolume, (basePath + "bgm_01.ogg").c_str(), 1, 1);
        } else if ( n == 1 ) {
            mbgm.open(&mBgmVolume, (basePath + "bgm_02.ogg").c_str(), 1, 1);
        } else if ( n == 2 ) {
            mbgm.open(&mBgmVolume, (basePath + "bgm_03.ogg").c_str(), 1, 1);
        }
        mbgm.play();
    }
    void stopBGM() {
        mbgm.stop();
    }
    void updateBgmVolume() {
        mBgmVolume = mMusicMuted ? 0.0f : mVolume * mMusicRatio;
        mbgm.setVolume(mBgmVolume);
    }
    void setVolume( float volume ) {
        mVolume = volume;
        if ( mVolume < 0.0 ) mVolume = 0;
        if ( mVolume > 1.0 ) mVolume = 1.0f;
        updateBgmVolume();
    }
    void setVolumeAdd( float add ) {
        mVolume += add;
        if ( mVolume < 0.0 ) mVolume = 0;
        if ( mVolume > 1.0 ) mVolume = 1.0f;
        updateBgmVolume();
    }
    void setMusicVolumeAdd( float add ) {
        mMusicRatio += add;
        if ( mMusicRatio < 0.0f ) mMusicRatio = 0;
        if ( mMusicRatio > 1.0f ) mMusicRatio = 1.0f;
        if ( mMusicRatio > 0.0f ) mMusicMuted = false;
        updateBgmVolume();
    }
    void toggleMusicMute() {
        mMusicMuted = ! mMusicMuted;
        updateBgmVolume();
    }
    float getMusicVolume() const {
        return mMusicRatio;
    }
    bool isMusicMuted() const {
        return mMusicMuted;
    }
    float getVolume() const {
        return mVolume;
    }
    friend class sound;
public:
    sound mSFX_move;
    sound mSFX_hold;
    sound mSFX_rotate;
    sound mSFX_softdrop;
    sound mSFX_harddrop;
    sound mSFX_lockdown;
    sound mSFX_lineattack;
    sound mSFX_clears[4];
    sound mSFX_combo[20];
    sound mSFX_tspin[4];
    sound mSFX_b2b_tspin[4];
    sound mSFX_b2b_tetris;
    sound mSFX_ko;
    sound mSFX_gameover;
    sound mSFX_pc;
    sound mbgm;
    float mVolume;
    float mMusicRatio;
    bool mMusicMuted;
    float mBgmVolume;
    ma_engine* mEngine;
    bool mReady;
};