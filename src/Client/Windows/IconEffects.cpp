#include "IconEffects.hpp"

ccColor3B EffectUI::getColourForSelected(int mode, bool player2) // bri`ish
{
    // 0 : primary, 1 : secondary : 2 : glow : 3 : trail : 4 : wave trail
    int sel = 0;
    float v = ColourUtility::va;

    v *= speeds[mode];

    switch (mode)
    {
        case 0:
            sel = primary;
            break;
        case 1:
            sel = secondary;
            break;
        case 2:
            sel = glow;
            break;
        case 3:
            sel = trail;
            break;
        case 4:
            sel = waveTrail;
            break;

        default:
            break;
    }

    auto gameManager = GameManager::get();

    if (sel == 0)
    {
        switch (mode)
        {
            case 0:
                return gameManager->colorForIdx(gameManager->m_playerColor.value());
            case 1:
                return gameManager->colorForIdx(gameManager->m_playerColor2.value());
            case 2:
                return gameManager->colorForIdx(gameManager->m_playerGlowColor.value());
            case 3:
                return gameManager->colorForIdx(sameDual ? gameManager->m_playerColor2.value() : (player2 ? gameManager->m_playerColor.value() : gameManager->m_playerColor2.value()));
            default:
                return gameManager->colorForIdx(sameDual ? gameManager->m_playerColor.value() : (player2 ? gameManager->m_playerColor2.value() : gameManager->m_playerColor.value()));
        }
    }

    if (sel == 1)
        return ColourUtility::getChromaColour(abs(v));

    if (sel == 2)
        return ColourUtility::getPastelColour(abs(v));

    if (sel == 3)
    {
        auto fadeIn = fmt::format("fadeColour1{}", mode);
        auto fadeOut = fmt::format("fadeColour2{}", mode);

        ccColor3B in = Mod::get()->getSavedValue<ccColor3B>(fadeIn, {0, 0, 0});
        ccColor3B out = Mod::get()->getSavedValue<ccColor3B>(fadeOut, {255, 255, 255});

        return ColourUtility::lerpColour(in, out, (sinf(v * 3) + 1) / 2);
        //fade
    }

    if (sel == 4)
    {
        //custom colour
    }

    return {0, 0, 0};
}

constexpr std::array mods =
{
    "rooot.custom-gamemode-colors",
    "gdemerald.custom_icon_colors",
    "capeling.coloured-wave-trail",
    "weebify.separate_dual_icons",
    "naxrin.progress_bar_color",
    "naxrin.rgb_icons",
    "asaki_zuki.same_dual_color",
    "saumondeluxe.rainbow_icon",
    "terma.ambienticons",
    "acaruso.pride",
    "the_bearodactyl.gay-wave-trail"
};

bool EffectUI::getIncompatibleModLoaded()
{
    for (auto mod : mods)
    {
        if (Loader::get()->isModLoaded(mod))
        {
            return true;
        }
    }
    
    return false;
}

std::string EffectUI::getIncompatibleMods()
{
    std::stringstream ss;

    for (auto mod : mods)
    {
        if (auto mod2 = Loader::get()->getLoadedMod(mod))
        {
            ss << "<cp>";
            ss << mod2->getName();
            ss << "</c>\n";
        }
    }

    return ss.str();
}

void EffectUI::updateValues()
{
    primary = Mod::get()->getSavedValue<int>(fmt::format("selColour{}", 0), 0);
    secondary = Mod::get()->getSavedValue<int>(fmt::format("selColour{}", 1), 0);
    glow = Mod::get()->getSavedValue<int>(fmt::format("selColour{}", 2), 0);
    trail = Mod::get()->getSavedValue<int>(fmt::format("selColour{}", 3), 0);
    waveTrail = Mod::get()->getSavedValue<int>(fmt::format("selColour{}", 4), 0);

    sameDual = Mod::get()->getSavedValue<bool>("same-dual");

    // cache values for performance
    for (int i = 0; i < 5; i++)
    {
        speeds[i] = Mod::get()->getSavedValue<float>(fmt::format("icon-effect-speed_{}", i), 1);
    }
}

class $modify (GJBaseGameLayer)
{
    virtual void update(float p0)
    {
        auto plr1 = EffectUI::getColourForSelected(0);
        auto plr2 = EffectUI::getColourForSelected(1);

        if (m_player1)
        {
            m_player1->setColor(plr1);
            m_player1->setSecondColor(plr2);
            m_player1->m_glowColor = EffectUI::getColourForSelected(2);
            m_player1->updateGlowColor();
            m_player1->m_regularTrail->setColor(EffectUI::getColourForSelected(3));
            m_player1->m_waveTrail->setColor(EffectUI::getColourForSelected(4));
        }

        if (m_player2)
        {
            if (!EffectUI::sameDual)
            {
                m_player2->setColor(EffectUI::getColourForSelected(1, true));
                m_player2->setSecondColor(EffectUI::getColourForSelected(0, true));
                m_player2->m_glowColor = EffectUI::getColourForSelected(2, true);
                m_player2->updateGlowColor();
            }
            else
            {
                m_player2->setColor(EffectUI::getColourForSelected(0, true));
                m_player2->setSecondColor(EffectUI::getColourForSelected(1, true));
                m_player2->m_glowColor = EffectUI::getColourForSelected(2, true);
                m_player2->updateGlowColor();
            }

            m_player2->m_regularTrail->setColor(EffectUI::getColourForSelected(3, true));
            m_player2->m_waveTrail->setColor(EffectUI::getColourForSelected(4, true));
        }

        if (m_effectManager)
        {
            if (auto action = m_effectManager->getColorAction(1005))
                action->m_color = plr1;
            
            if (auto action = m_effectManager->getColorAction(1006))
                action->m_color = plr2;
        }

        GJBaseGameLayer::update(p0);
    }

    static void onModify(auto& self) {
        EffectUI::_hook = self.getHook("GJBaseGameLayer::update").unwrap();
        (void)self.setHookPriority("GJBaseGameLayer::update", 69420);
    }
};

class $modify (MenuLayer)
{
    bool init()
    {
        if (EffectUI::getIncompatibleModLoaded())
        {
            EffectUI::_hook->setAutoEnable(false);
            (void)EffectUI::_hook->disable();

            log::error("Incompatible mod loaded, disabling icon effects");
        }

        return MenuLayer::init();
    }
};

$execute
{
    EffectUI::updateValues();
};