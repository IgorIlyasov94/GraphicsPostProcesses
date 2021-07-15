#include "UISystem.h"

Graphics::UISystem::UISystem()
{

}

Graphics::UISystem::~UISystem()
{

}

Graphics::UISpriteId Graphics::UISystem::AddSprite(const SpriteUI& sprite)
{
	sprites.push_back(sprite);

	return UISpriteId(sprites.size() - 1);
}

Graphics::UIButtonId Graphics::UISystem::AddButton(const SpriteUI& sprite/*, const Script& script*/)
{
	spriteButtons.push_back(sprite);
	//buttonEvents.push_back(script);

	return UIButtonId(spriteButtons.size() - 1);
}

Graphics::UITextId Graphics::UISystem::AddText(const TextUI& text)
{
	texts.push_back(text);

	return UITextId(texts.size() - 1);
}

void Graphics::UISystem::ExecuteScripts(size_t mouseX, size_t mouseY)
{
	/*float2 mousePosition = { 2.0f * mouseX / static_cast<float>(GraphicsSettings::GetResolutionX()) - 1.0f,
		1.0f - 2.0f * mouseY / static_cast<float>(GraphicsSettings::GetResolutionY()) };

	for (size_t buttonId = 0; buttonId < spriteButtons.size(); buttonId++)
		if (spriteButtons[buttonId].IsActive())
			if (spriteButtons[buttonId].PointInsideMesh(mousePosition))
				buttonEvents[buttonId].Execute();*/
}

void Graphics::UISystem::Draw(ID3D12GraphicsCommandList* commandList) const
{
	for (auto& sprite : sprites)
		if (sprite.IsActive())
			sprite.Draw(commandList);

	for (auto& button : spriteButtons)
		if (button.IsActive())
			button.Draw(commandList);

	for (auto& text : texts)
		if (text.IsActive())
			text.Draw(commandList);
}
