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

    class AIState;

    class NPC :
        public Actor
    {
    public:
        virtual void Shutdown() override;
        virtual void Tick() override;
        virtual bool AddToViewList(const id_t id, const bool bSendMove = false) override;
        virtual bool RemoveFromViewList(const id_t id) override;

        void OnHit(const id_t hitterID);
        void PushState(AIState* newState);
        void ChangeState(AIState* newState);
        void BackToPreviousState();

    public:
        bool IsHostile() const { return mbHostile; }
        NpcBehaviorType GetBehaviorType() const { return mBehaviorType; }
        NpcMoveType GetMoveType() const { return mMoveType; }
        id_t GetTargetID() const { return mTargetID; }

        void SetHostile(const bool value) { mbHostile = value; }
        void SetBehaviorType(const NpcBehaviorType value) { mBehaviorType = value; }
        void SetMoveType(const NpcMoveType value) { mMoveType = value; }
        void SetTargetID(const id_t value) { mTargetID = value; }

    private:
        bool mbHostile = false;
        NpcBehaviorType mBehaviorType = NpcBehaviorType::NONE;
        NpcMoveType mMoveType = NpcMoveType::NONE;
        id_t mTargetID = INVALID_VALUE;

        std::shared_ptr<AIState> mCurrentState = nullptr;
        std::shared_ptr<AIState> mPrevState = nullptr;
    };
}

