AnimStateMachine = {
    current_state = "Idle",
    elapsed = 0.0,

    -- 상태 → 애니메이션 매핑
    animations = {
        Idle = { name = "Anim_Idle", blend = 0.2 },
        Walk = { name = "Anim_Walk", blend = 0.1 },
        Jump = { name = "Anim_Jump", blend = 0.15 }
    },

    -- 상태 전이 조건
    transitions = {
        Idle = function(self)
            if self:GetSpeed() > 0 then return "Walk" end
        end,
        Walk = function(self)
            if self:GetSpeed() <= 0 then return "Idle" end
            if self:IsJumping() then return "Jump" end
        end,
        Jump = function(self)
            if not self:IsJumping() then return "Idle" end
        end
    },

    GetSpeed = function(self)
        return self.owner:GetSpeed()
    end,

    IsJumping = function(self)
        return self.owner:IsJumping()
    end,

    Update = function(self, dt)
        self.elapsed = self.elapsed + dt

        local transitionFn = self.transitions[self.current_state]
        if transitionFn then
            local new_state = transitionFn(self)
            if new_state and new_state ~= self.current_state then
                self.current_state = new_state
                self.elapsed = 0.0
            end
        end

        return self.animations[self.current_state].name
    end
}