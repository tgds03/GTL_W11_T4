AnimFSM = {
    current = "Idle",
    Update = function(self, dt)

        self.current = "Contents/Fbx/Capoeira.fbx"


        -- local speed = self.owner:GetVelocity():Length()

        -- if speed < 10 then
        --     self.current = "Idle"
        -- elseif speed < 300 then
        --     self.current = "Walk"
        -- else
        --     self.current = "Run"
        -- end

        return {
            anim = self.current,
            blend = 0.25
        }
    end
}

return AnimFSM