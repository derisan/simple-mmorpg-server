#pragma once

#include "Actor.h"

namespace mk
{
    enum class NpcBehaviorType
    {
        PEACE,
        AGGRO,
    };

    enum class NpcMoveType
    {
        FIXED,
        ROAMING,
    };

    class NPC :
        public Actor
    {
    public:
        virtual void AddToViewList(const int id) override;
        virtual void RemoveFromViewList(const int id) override;

    public:
        bool GetActive() const { return mbActive; }
        bool IsHostile() const { return mbHostile; }
        NpcBehaviorType GetBehaviorType() const { return mBehaviorType; }
        NpcMoveType GetMoveType() const { mMoveType; }

        void SetActive(const bool value) { mbActive = value; }
        void SetHostile(const bool value) { mbHostile = value; }
        void SetBehaviorType(const NpcBehaviorType value) { mBehaviorType = value; }
        void SetMoveType(const NpcMoveType value) { mMoveType = value; }

    private:
        bool mbActive = false;
        bool mbHostile = false;
        NpcBehaviorType mBehaviorType = NpcBehaviorType::PEACE;
        NpcMoveType mMoveType = NpcMoveType::FIXED;
    };
}

