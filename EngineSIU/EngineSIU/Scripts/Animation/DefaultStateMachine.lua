AnimFSM = {
    current = "Idle",
    Update = function(self, dt)

        -- self.current = "Contents/Fbx/Capoeira.fbx"

        local Exciting = self.Owner.Exciting

        if Exciting < 2 then
            self.current = "Contents/Fbx/Capoeira.fbx"
        else
            self.current = "Contents/Fbx/Twerkbin.fbx"
        end

        return {
            anim = self.current,
            blend = 0.25
        }
    end
}

return AnimFSM