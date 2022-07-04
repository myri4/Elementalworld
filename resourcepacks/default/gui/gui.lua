function Startup()
	maincontext = rmlui.contexts["main"]
	--maincontext:LoadDocument("resourcepacks/default/gui/gui.html")
	rmlui.LoadFontFace('resourcepacks/default/font/Minecraft.ttf')
end

Startup()

--element = context.documents["resourcepacks/default/gui/gui.html"]:GetElementById('chatTextbox')
--element:AddEventListener("keyup", checkKeyPress, false)