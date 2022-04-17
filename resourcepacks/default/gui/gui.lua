function Init(document)
    print('hello')
	element = document:GetElementById('chatTextbox')
	element:AddEventListener('click', "print('Line 1') print('Line 2')", true)
end