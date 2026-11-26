#if defined _mappulse_settings_included
    #endinput
#endif
#define _mappulse_settings_included // Zastita od visestrukog ukljucivanja istog fajla.

// OPSTA PODESAVANJA

#if !defined MPL_START_ENABLED
    #define MPL_START_ENABLED             (1)                    // 1 = automatski ukljucuje individualnu optimizaciju pri pokretanju.
#endif
#if !defined MPL_USE_PAWNCMD
    #define MPL_USE_PAWNCMD               (1)                    // 1 = Pawn.CMD, 0 = standardni OnPlayerCommandText bez dodatnog plugina.
#endif
#if !defined MPL_DIALOG_BRIDGE
    #define MPL_DIALOG_BRIDGE             (0)                    // 1 = koristi postojeci gamemode callback MapPulse_ShowReport umesto direktnog dialoga.
#endif
#if !defined MPL_DIALOG_ID
    #define MPL_DIALOG_ID                 (32740)                // ID administratorskog dialoga; rezervisi ga u gamemode-u.
#endif
#if !defined MPL_COLOR
    #define MPL_COLOR                     (0xA9DEFFFF)           // RGBA boja poruka MapPulse-a u chatu.
#endif
#if !defined MPL_PRINT_BANNER
    #define MPL_PRINT_BANNER              (1)                    // 1 = prikazuje ASCII logo pri pokretanju.
#endif
#if !defined MPL_SAVE_RESULTS
    #define MPL_SAVE_RESULTS              (1)                    // 1 = cuva rezultate A/B testova u scriptfiles folderu.
#endif
#if !defined MPL_RESULTS_FILE
    #define MPL_RESULTS_FILE              "mappulse_performance_results.log" // Naziv log fajla u scriptfiles folderu.
#endif

// PROVERA ZAVISNOSTI

#if !defined MPL_STREAMER_RECOMMENDED
    #define MPL_STREAMER_RECOMMENDED      (0x296)                // Preporucena verzija streamer.inc u heksadecimalnom formatu (0x296 = 2.9.6).
#endif
#if !defined MPL_PAWNCMD_RECOMMENDED
    #define MPL_PAWNCMD_RECOMMENDED       (0x030400)             // Preporucena verzija Pawn.CMD include-a (0x030400 = 3.4.0).
#endif
#if !defined MPL_DEP_STRICT
    #define MPL_DEP_STRICT                (0)                    // 1 = blokira optimizaciju sa starim/nepoznatim include-om; 0 = upozorenje.
#endif

// PERFORMANSE I RASPORED OBRADA

#if !defined MPL_TICK
    #define MPL_TICK                      (100)                  // Interval zajednickog timera u milisekundama.
#endif
#if !defined MPL_PLAYER_MS
    #define MPL_PLAYER_MS                 (1500)                 // Najkraci razmak izmedju obrada igraca u milisekundama.
#endif
#if !defined MPL_IDLE_MS
    #define MPL_IDLE_MS                   (2500)                 // Razmak obrada igraca koji miruje i nema aktivnu izmenu radijusa (ms).
#endif
#if !defined MPL_WORK_PER_TICK
    #define MPL_WORK_PER_TICK             (16)                   // Maksimalan broj pregledanih aktivnih slotova po prolazu timera.
#endif
#if !defined MPL_CPU_BUDGET_MS
    #define MPL_CPU_BUDGET_MS             (2)                    // Meki vremenski budzet jednog prolaza (ms); pojedinacni poziv moze ga prekoraciti.
#endif
#if !defined MPL_ADJUST_MS
    #define MPL_ADJUST_MS                 (6000)                 // Minimalan razmak izmedju redovnih promena radijusa (ms).
#endif
#if !defined MPL_STABLE_SAMPLES
    #define MPL_STABLE_SAMPLES            (3)                    // Broj uzastopnih uzoraka istog nivoa brzine pre redovne promene.
#endif

// KRETANJE I DETEKCIJA SKOKOVA

#if !defined MPL_MAX_GAP_MS
    #define MPL_MAX_GAP_MS                (15000)                // Najveci razmak uzoraka pre nego sto se kretanje smatra diskontinuitetom (ms).
#endif
#if !defined MPL_JUMP_DISTANCE
    #define MPL_JUMP_DISTANCE             (150.0)                // Minimalni prag udaljenosti za detekciju skoka/teleporta (metri).
#endif
#if !defined MPL_JUMP_METERS_PER_MS
    #define MPL_JUMP_METERS_PER_MS        (0.12)                 // Dodatni prag skoka prema vremenu uzorkovanja (metri po ms).
#endif
#if !defined MPL_SPEED_WEIGHT
    #define MPL_SPEED_WEIGHT              (0.5)                  // Tezina novog uzorka kvadrata brzine (0.0 do 1.0).
#endif
#if !defined MPL_IDLE_SPEED
    #define MPL_IDLE_SPEED                (2.0)                  // Brzina do koje se igrac smatra mirujucim (km/h).
#endif
#if !defined MPL_WALK_SPEED
    #define MPL_WALK_SPEED                (4.0)                  // Minimalna brzina za adaptivnu obradu radijusa (km/h).
#endif
#if !defined MPL_DRIVE_SPEED
    #define MPL_DRIVE_SPEED               (25.0)                 // Prag brzine za srednji nivo radijusa (km/h).
#endif
#if !defined MPL_FAST_SPEED
    #define MPL_FAST_SPEED                (90.0)                 // Prag brzine za najvisi nivo radijusa (km/h).
#endif

// INDIVIDUALNI RADIJUS I KAPACITET

#if !defined MPL_EPS
    #define MPL_EPS                       (0.005)                // Tolerancija pri poredjenju multiplikatora i detekciji spoljne izmene.
#endif
#if !defined MPL_MIN_CHANGE
    #define MPL_MIN_CHANGE                (0.025)                // Minimalna razlika multiplikatora potrebna za upis nove vrednosti.
#endif
#if !defined MPL_MAX_MULTIPLIER
    #define MPL_MAX_MULTIPLIER            (1.5)                  // Gornja granica povecanja multiplikatora; ne obara veci bazni radijus.
#endif
#if !defined MPL_PRESSURE_HIGH
    #define MPL_PRESSURE_HIGH             (85)                   // Procenat popunjenosti pri kom se vraca originalni radijus.
#endif
#if !defined MPL_PRESSURE_LOW
    #define MPL_PRESSURE_LOW              (60)                   // Procenat popunjenosti do kog mora pasti zasicenje pre ponovnog povecanja.
#endif
#if !defined MPL_TARGET_OCCUPANCY
    #define MPL_TARGET_OCCUPANCY          (75)                   // Ciljna popunjenost (%) za matematicku procenu dozvoljenog povecanja.
#endif
#if !defined MPL_WALK_FACTOR
    #define MPL_WALK_FACTOR               (1.0)                  // Mnozilac baznog radijusa za prvi nivo brzine.
#endif
#if !defined MPL_DRIVE_FACTOR
    #define MPL_DRIVE_FACTOR              (1.10)                 // Mnozilac baznog radijusa za srednji nivo brzine.
#endif
#if !defined MPL_FAST_FACTOR
    #define MPL_FAST_FACTOR               (1.20)                 // Mnozilac baznog radijusa za najvisi nivo brzine.
#endif

// GLOBALNA POLITIKA STATICKIH MAPA

#if !defined MPM_ENABLE
    #define MPM_ENABLE                    (1)                    // 1 = primenjuje politiku na podrzane staticke objekte pri kreiranju.
#endif
#if !defined MPM_ENABLE_PRIORITY
    #define MPM_ENABLE_PRIORITY           (1)                    // 1 = automatski dodeljuje prioritet odabranim modelima sa izvornim prioritetom 0.
#endif
#if !defined MPM_PRIORITY_HIGH
    #define MPM_PRIORITY_HIGH             (3)                    // Prioritet modela iz MPM_HIGH_MODELS.
#endif
#if !defined MPM_PRIORITY_MEDIUM
    #define MPM_PRIORITY_MEDIUM           (2)                    // Prioritet modela iz MPM_MEDIUM_MODELS.
#endif
#if !defined MPM_HIGH_MODELS
    #define MPM_HIGH_MODELS               18762,18766,19378,19381,19447,19448 // ID-jevi modela visokog prioriteta, odvojeni zarezima.
#endif
#if !defined MPM_MEDIUM_MODELS
    #define MPM_MEDIUM_MODELS             19362,19429,19433,19435,19436,19438,19446,19456,19457,19458,19459 // ID-jevi modela srednjeg prioriteta, odvojeni zarezima; bez duplikata sa visokom listom.
#endif
#if !defined MPM_TRIM_DECOR
    #define MPM_TRIM_DECOR                (1)                    // 1 = dozvoljava skracenje streaming udaljenosti odabranog dekorativnog modela.
#endif
#if !defined MPM_DECOR_MODEL
    #define MPM_DECOR_MODEL               (14400)                // ID dekorativnog modela za selektivno skracenje udaljenosti.
#endif
#if !defined MPM_DECOR_MIN_SD
    #define MPM_DECOR_MIN_SD              (120.0)                // Donja granica streaming udaljenosti dekoracije (metri).
#endif
#if !defined MPM_DECOR_MAX_SD
    #define MPM_DECOR_MAX_SD              (500.0)                // Najveca originalna streaming udaljenost dekoracije za primenu pravila (metri).
#endif
#if !defined MPM_DECOR_FACTOR
    #define MPM_DECOR_FACTOR              (0.85)                 // Mnozilac originalne streaming udaljenosti dekoracije; 0.85 = 85%.
#endif
#if !defined MPM_DECOR_MIN_CHANGE
    #define MPM_DECOR_MIN_CHANGE          (5.0)                  // Minimalno potrebno skracenje streaming udaljenosti (metri).
#endif

// ADMINISTRATORSKI A/B TESTOVI

#if !defined MPL_AB_MIN_SAMPLES
    #define MPL_AB_MIN_SAMPLES            (15)                   // Minimalni broj uzoraka u svakom A/B testu za uporedivost.
#endif
#if !defined MPL_AB_ROUTE_REL
    #define MPL_AB_ROUTE_REL              (0.15)                 // Dozvoljena relativna razlika duzine rute (0.15 = 15%).
#endif
#if !defined MPL_AB_ROUTE_ABS
    #define MPL_AB_ROUTE_ABS              (10.0)                 // Dodatna apsolutna tolerancija duzine rute (metri).
#endif
#if !defined MPL_AB_ENDPOINT_DISTANCE
    #define MPL_AB_ENDPOINT_DISTANCE      (50.0)                 // Najveca udaljenost izmedju odgovarajucih pocetnih i krajnjih tacaka (metri).
#endif
#if !defined MPL_AB_SPEED_REL
    #define MPL_AB_SPEED_REL              (0.20)                 // Dozvoljena relativna razlika maksimalne brzine (0.20 = 20%).
#endif
#if !defined MPL_AB_SPEED_ABS
    #define MPL_AB_SPEED_ABS              (5.0)                  // Dodatna tolerancija maksimalne brzine (km/h).
#endif
#if !defined MPL_AB_DELTA_PP
    #define MPL_AB_DELTA_PP               (2.0)                  // Prag bitne razlike punog kapaciteta (procentni poeni).
#endif

#if MPL_TICK <= 0 || MPL_PLAYER_MS <= 0 || MPL_WORK_PER_TICK <= 0
    #error MapPulse: scheduler values must be positive
#endif

#if MPL_PRESSURE_LOW < 0 || MPL_PRESSURE_HIGH > 100 || MPL_PRESSURE_LOW >= MPL_PRESSURE_HIGH
    #error MapPulse: invalid pressure thresholds
#endif

#if MPL_CPU_BUDGET_MS < 1 || MPL_STABLE_SAMPLES < 1 || MPL_IDLE_MS < MPL_PLAYER_MS
    #error MapPulse: invalid performance settings
#endif

#if MPL_TARGET_OCCUPANCY < MPL_PRESSURE_LOW || MPL_TARGET_OCCUPANCY > MPL_PRESSURE_HIGH
    #error MapPulse: invalid occupancy target
#endif

#if MPL_AB_MIN_SAMPLES < 1
    #error MapPulse: invalid A/B sample count
#endif

#include <streamer>

new MPD_StreamerStatus;

stock MPD_CheckStreamer()
{
    if (MPD_StreamerStatus) return MPD_StreamerStatus;
    #if defined Streamer_IncludeFileVersion
        new version=Streamer_IncludeFileVersion;
        MPD_StreamerStatus=(version>=MPL_STREAMER_RECOMMENDED)?1:-1;
        printf("[MapPulse] Streamer include: 0x%x | recommended: 0x%x",version,MPL_STREAMER_RECOMMENDED);
        if (MPD_StreamerStatus<0)
            print("[MapPulse][WARNING] Update streamer.inc and the matching Streamer plugin, then recompile.");
        else
            print("[MapPulse][OK] Streamer include meets the configured baseline.");
    #else
        MPD_StreamerStatus=-1;
        print("[MapPulse][WARNING] Streamer include version unavailable. Update the include and matching plugin.");
    #endif
    #if MPL_DEP_STRICT
        if (MPD_StreamerStatus<0) print("[MapPulse][WARNING] Strict mode: Streamer-dependent optimization blocked for this script.");
    #endif
    if (MPD_StreamerStatus<0)
        print("[MapPulse] https://github.com/samp-incognito/samp-streamer-plugin/releases");
    print("[MapPulse] Streamer binary version: not exposed by Pawn. Check its startup banner and mismatch warnings.");
    return MPD_StreamerStatus;
}

stock bool:MPD_AllowMapPolicy()
{
    if (!MPD_StreamerStatus) MPD_CheckStreamer();
    #if MPL_DEP_STRICT
        return MPD_StreamerStatus>0;
    #else
        return true;
    #endif
}

stock bool:MPD_CheckLive(pcVersion)
{
    new streamerStatus=MPD_CheckStreamer();
    new bool:headersOK=(streamerStatus>0);
    #if MPL_USE_PAWNCMD
        printf("[MapPulse] Pawn.CMD include: 0x%x | recommended: 0x%x",pcVersion,MPL_PAWNCMD_RECOMMENDED);
        if (pcVersion<MPL_PAWNCMD_RECOMMENDED)
        {
            headersOK=false;
            print("[MapPulse][WARNING] Update Pawn.CMD include and matching plugin/component, then recompile.");
            print("[MapPulse] https://github.com/katursis/Pawn.CMD/releases");
        }
        else print("[MapPulse][OK] Pawn.CMD include meets the configured baseline.");
        print("[MapPulse] Pawn.CMD binary version: not exposed by Pawn. Check its startup banner and mismatch warnings.");
    #else
        #pragma unused pcVersion
        print("[MapPulse][SKIP] Pawn.CMD disabled; native Pawn commands selected.");
    #endif
    #if MPL_DIALOG_BRIDGE
        print("[MapPulse][INFO] Dialog bridge enabled. Host callback MapPulse_ShowReport must be installed and tested.");
    #else
        print("[MapPulse][INFO] Native dialogs selected; reserve MPL_DIALOG_ID in the gamemode.");
    #endif
    #if MPL_DEP_STRICT
        if (!headersOK)
        {
            print("[MapPulse][ERROR] Dependency baseline not met; individual optimization disabled by strict mode.");
            return false;
        }
    #else
        if (!headersOK) print("[MapPulse][WARNING] Old or unknown include; continuing with API probe (strict mode disabled).");
    #endif
    new Float:radius;
    new tick=Streamer_GetTickRate(), limit=Streamer_GetVisibleItems(STREAMER_TYPE_OBJECT);
    if (tick<=0 || limit<=0 || !Streamer_GetRadiusMultiplier(STREAMER_TYPE_OBJECT,radius) || radius<=0.0)
    {
        print("[MapPulse][ERROR] Streamer read-only API probe failed. Check plugin loading and matching versions.");
        return false;
    }
    printf("[MapPulse][OK] Streamer API probe: tick=%d object limit=%d radius=%.2f",tick,limit,radius);
    return true;
}
