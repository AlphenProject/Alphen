﻿#include "spammers.h"

void spammers::clan_tag()
{
    auto apply = [](const char* tag) -> void
    {
        using Fn = int(__fastcall*)(const char*, const char*);
        static auto fn = reinterpret_cast<Fn>(util::FindSignature(crypt_str("engine.dll"), crypt_str("53 56 57 8B DA 8B F9 FF 15")));

        fn(tag, tag);
    };

    static auto removed = false;

    if (!g_cfg.misc.clantag_spammer && !removed)
    {
        removed = true;
        apply(crypt_str(""));
        return;
    }

    auto nci = m_engine()->GetNetChannelInfo();

    if (!nci)
        return;

    static auto time = -1;

    auto ticks = TIME_TO_TICKS(nci->GetAvgLatency(FLOW_OUTGOING)) + (float)m_globals()->m_tickcount; //-V807
    auto intervals = 0.2f / m_globals()->m_intervalpertick;

    if (g_cfg.misc.clantag_spammer && g_cfg.misc.clantags_mode == 0)
    {
        auto main_time = (int)(ticks / intervals) % 16;

        if (main_time != time && !m_clientstate()->iChokedCommands)
        {
            auto tag = crypt_str("");

            switch (main_time)
            {
            case 0:
                tag = crypt_str("A|"); //>V1037
                break;
            case 1:
                tag = crypt_str("Al|"); //>V1037
                break;
            case 2:
                tag = crypt_str("Alp|"); //>V1037
                break;
            case 3:
                tag = crypt_str("Alph|"); //>V1037
                break;
            case 4:
                tag = crypt_str("Alphe|");
                break;
            case 5:
                tag = crypt_str("Alphen"); //>V1037
                break;
            case 6:
                tag = crypt_str("Alphen");
                break;
            case 7:
                tag = crypt_str("Alphen");
                break;
            case 8:
                tag = crypt_str("Alphe|");
                break;
            case 9:
                tag = crypt_str("Alph|");
                break;
            case 10:
                tag = crypt_str("Alph|");
                break;
            case 11:
                tag = crypt_str("Alp|");
                break;
            case 12:
                tag = crypt_str("Al|");
                break;
            case 13:
                tag = crypt_str("A|");
                break;
            case 14:
                tag = crypt_str("|");
                break;
            case 15:
                tag = crypt_str("");
                break;
            }

            apply(tag);
            time = main_time;
        }
        removed = false;
    }
    else if (g_cfg.misc.clantag_spammer && g_cfg.misc.clantags_mode == 1)
    {
        auto main_time = (int)(ticks / intervals) % 28;

        if (main_time != time && !m_clientstate()->iChokedCommands)
        {
            auto tag = crypt_str("");

            switch (main_time)
            {
            case 0:  
                tag = crypt_str("                  ");
                break;
            case 1:  
                tag = crypt_str("                 g"); 
                break;
            case 2:  
                tag = crypt_str("                ga"); 
                break;
            case 3:  
                tag = crypt_str("               gam"); 
                break;
            case 4:  
                tag = crypt_str("              game");
                break;
            case 5:  
                tag = crypt_str("             games"); 
                break;
            case 6:  
                tag = crypt_str("            gamese"); 
                break;
            case 7:  
                tag = crypt_str("           gamesen"); 
                break;
            case 8:  
                tag = crypt_str("          gamesens"); 
                break;
            case 9: 
                tag = crypt_str("         gamesense"); 
                break;
            case 10:
                tag = crypt_str("        gamesense "); 
                break;
            case 11:
                tag = crypt_str("       gamesense  "); 
                break;
            case 12:
                tag = crypt_str("      gamesense   ");
                break;
            case 13:
                tag = crypt_str("     gamesense    "); 
                break;
            case 14:
                tag = crypt_str("    gamesense     "); 
                break;
            case 15:
                tag = crypt_str("   gamesense      ");
                break;
            case 16:
                tag = crypt_str("  gamesense       ");
                break;
            case 17:
                tag = crypt_str(" gamesense        ");
                break;
            case 18:
                tag = crypt_str("gamesense         ");
                break;
            case 19:
                tag = crypt_str("amesense          ");
                break;
            case 20:
                tag = crypt_str("mesense           ");
                break;
            case 21:
                tag = crypt_str("esense            ");
                break;
            case 22:
                tag = crypt_str("sense             ");
                break;
            case 23:
                tag = crypt_str("ense              ");
                break;
            case 24:
                tag = crypt_str("nse               ");
                break;
            case 25:
                tag = crypt_str("se                ");
                break;
            case 26:
                tag = crypt_str("e                 ");
                break;
            case 27:
                tag = crypt_str("                  ");
                break;
            }

            apply(tag);
            time = main_time;
        }
        removed = false;
    }

        else if (g_cfg.misc.clantag_spammer && g_cfg.misc.clantags_mode == 2)
        {
            auto main_time = (int)(ticks / intervals) % 56;

            if (main_time != time && !m_clientstate()->iChokedCommands)
            {
                auto tag = crypt_str("");

                switch (main_time)
                {
                case 0: tag = crypt_str("                        "); break;
                case 1: tag = crypt_str("                       |"); break;
                case 2: tag = crypt_str("                      "); break;
                case 3: tag = crypt_str("                      N "); break;
                case 4: tag = crypt_str("                      N "); break;
                case 5: tag = crypt_str("                     N3 "); break;
                case 6: tag = crypt_str("                    Ne  "); break;
                case 7: tag = crypt_str("                   Ne\  "); break;
                case 8: tag = crypt_str("                  Ne\/  "); break;
                case 9: tag = crypt_str("                 Nev    "); break;
                case 10: tag = crypt_str("                Nev3    "); break;
                case 11: tag = crypt_str("               Neve     "); break;
                case 12: tag = crypt_str("              Neve|     "); break;
                case 13: tag = crypt_str("             Neve|2     "); break;
                case 14: tag = crypt_str("            Never|      "); break;
                case 15: tag = crypt_str("           Never|_      "); break;
                case 16: tag = crypt_str("          Neverl        "); break;
                case 17: tag = crypt_str("         Neverl0        "); break;
                case 18: tag = crypt_str("        Neverlo         "); break;
                case 19: tag = crypt_str("       Neverlo5         "); break;
                case 20: tag = crypt_str("      Neverlos          "); break;
                case 21: tag = crypt_str("     Neverlos3          "); break;
                case 22: tag = crypt_str("    Neverlose           "); break;
                case 23: tag = crypt_str("   Neverlose.           "); break;
                case 24: tag = crypt_str("  Neverlose.<           "); break;
                case 25: tag = crypt_str(" Neverlose.c<            "); break;
                case 26: tag = crypt_str("Neverlose.cc             "); break;
                case 27: tag = crypt_str("Neverlose.cc             "); break;
                case 28: tag = crypt_str(" Neverlose.c<            "); break;
                case 29: tag = crypt_str("  Neverlose.<            "); break;
                case 30: tag = crypt_str("   Neverlose.            "); break;
                case 31: tag = crypt_str("    Neverlose            "); break;
                case 32: tag = crypt_str("     Neverlos3           "); break;
                case 33: tag = crypt_str("      Neverlos           "); break;
                case 34: tag = crypt_str("       Neverlo_          "); break;
                case 35: tag = crypt_str("        Neverlo5         "); break;
                case 36: tag = crypt_str("         Neverlo         "); break;
                case 37: tag = crypt_str("          Neverl_        "); break;
                case 38: tag = crypt_str("           Never|0       "); break;
                case 39: tag = crypt_str("            Never|       "); break;
                case 40: tag = crypt_str("             Neve|2      "); break;
                case 41: tag = crypt_str("              Neve|      "); break;
                case 42: tag = crypt_str("               Neve      "); break;
                case 43: tag = crypt_str("                Nev3     "); break;
                case 44: tag = crypt_str("                 Nev     "); break;
                case 45: tag = crypt_str("                  Ne\/   "); break;
                case 46: tag = crypt_str("                   Ne\   "); break;
                case 47: tag = crypt_str("                    Ne   "); break;
                case 48: tag = crypt_str("                     N3  "); break;
                case 49: tag = crypt_str("                      N  "); break;
                case 50: tag = crypt_str("                      |\|"); break;
                case 51: tag = crypt_str("                         "); break;
                case 52: tag = crypt_str("                        |"); break;
                case 53: tag = crypt_str("                         "); break;
                case 54: tag = crypt_str("                         "); break;
                case 55: tag = crypt_str("                         "); break;
                }

                apply(tag);
                time = main_time;
            }
            removed = false;
            }

}