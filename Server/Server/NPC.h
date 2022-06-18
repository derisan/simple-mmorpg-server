#pragma once

#include "Actor.h"

namespace mk
{
    enum class NpcBehaviorType
    {
        NONE,
        PEACE,
        AGGRO,
    };

    enum class NpcMoveType
    {
        NONE,
        FIXED,
        ROAMING,
    };

    class NPC :
        public Actor
    {
    public:
        virtual void Shutdown() override;
        virtual void Tick() override;
        virtual bool AddToViewList(const id_t id, const bool bSendMove = false) override;
        virtual bool RemoveFromViewList(const id_t id) override;

    public:
        bool IsHostile() const { return mbHostile; }
        NpcBehaviorType GetBehaviorType() const { return mBehaviorType; }
        NpcMoveType GetMoveType() const { return mMoveType; }

        void SetHostile(const bool value) { mbHostile = value; }
        void SetBehaviorType(const NpcBehaviorType value) { mBehaviorType = value; }
        void SetMoveType(const NpcMoveType value) { mMoveType = value; }

    private:
        bool mbHostile = false;
        NpcBehaviorType mBehaviorType = NpcBehaviorType::NONE;
        NpcMoveType mMoveType = NpcMoveType::NONE;
    };
}

