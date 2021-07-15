#pragma once

#include "ResourceManager.h"
#include "SpriteUI.h"
#include "TextUI.h"
#include "GraphicsSettings.h"

namespace Graphics
{
	using UISpriteId = typename ResourceId<21>;
	using UIButtonId = typename ResourceId<22>;
	using UITextId = typename ResourceId<23>;

	class UISystem
	{
	public:
		UISystem();
		~UISystem();

		UISpriteId AddSprite(const SpriteUI& sprite);
		UIButtonId AddButton(const SpriteUI& sprite/*, const Script& script*/);
		UITextId AddText(const TextUI& text);

		void ExecuteScripts(size_t mouseX, size_t mouseY);
		void Draw(ID3D12GraphicsCommandList* commandList) const;

	private:
		std::vector<SpriteUI> sprites;
		std::vector<SpriteUI> spriteButtons;
		//std::vector<Script> buttonEvents;
		std::vector<TextUI> texts;

		ResourceManager& resourceManager = ResourceManager::GetInstance();
	};
}
