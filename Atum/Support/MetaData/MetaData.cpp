
#include "MetaData.h"

MetaData::MetaData()
{
	inited = false;
	widgets_inited = false;
}

void MetaData::Prepare(void* set_owner)
{
	if (!inited)
	{
		Init();
		inited = true;
	}

	owner = set_owner;

	for (int i = 0; i < properties.size(); i++)
	{
		Property& prop = properties[i];
		prop.value = (uint8_t*)owner + prop.offset;
	}
}

void MetaData::SetDefValuesPrepare()
{
	for (int i = 0; i < properties.size(); i++)
	{
		Property& prop = properties[i];

		if (prop.type == Boolean)
		{
			memcpy(prop.value, &prop.defvalue.boolean, sizeof(bool));
		}
		else
		if (prop.type == Integer)
		{
			memcpy(prop.value, &prop.defvalue.integer, sizeof(int));
		}
		else
		if (prop.type == Float)
		{
			memcpy(prop.value, &prop.defvalue.flt, sizeof(float));
		}
		else
		if (prop.type == String || prop.type == FileName)
		{
			*((std::string*)prop.value) = defStrings[prop.defvalue.string];
		}
		else
		if (prop.type == Clor)
		{
			memcpy(prop.value, prop.defvalue.color, sizeof(float) * 4);
		}
		else
		if (prop.type == Enum)
		{
			MetaDataEnum& enm = enums[prop.defvalue.enumIndex];

			int value = enm.values[enm.defIndex];
			memcpy(prop.value, &value, sizeof(int));
		}
	}
}

void MetaData::Load(JSONReader& reader)
{
	for (int i = 0; i < properties.size(); i++)
	{
		Property& prop = properties[i];

		if (prop.type == Boolean)
		{
			bool val;
			if (reader.Read(prop.propName.c_str(), val))
			{
				memcpy(prop.value, &val, sizeof(bool));
			}
		}
		else
		if (prop.type == Integer || prop.type == Enum)
		{
			int val;
			if (reader.Read(prop.propName.c_str(), val))
			{
				memcpy(prop.value, &val, sizeof(int));
			}
		}
		else
		if (prop.type == Float)
		{
			float val;
			if (reader.Read(prop.propName.c_str(), val))
			{
				memcpy(prop.value, &val, sizeof(float));
			}
		}
		else
		if (prop.type == String || prop.type == FileName)
		{
			reader.Read(prop.propName.c_str(), *((std::string*)prop.value));
		}
		else
		if (prop.type == Clor)
		{
			reader.Read(prop.propName.c_str(), *((Color*)prop.value));
		}
	}
}

void MetaData::Save(JSONWriter& writer)
{
	for (int i = 0; i < properties.size(); i++)
	{
		Property& prop = properties[i];

		if (prop.type == Boolean)
		{
			writer.Write(prop.propName.c_str(), *((bool*)prop.value));
		}
		else
		if (prop.type == Integer || prop.type == Enum)
		{
			writer.Write(prop.propName.c_str(), *((int*)prop.value));
		}
		else
		if (prop.type == Float)
		{
			writer.Write(prop.propName.c_str(), *((float*)prop.value));
		}
		else
		if (prop.type == String || prop.type == FileName)
		{
			writer.Write(prop.propName.c_str(), ((std::string*)prop.value)->c_str());
		}
		else
		if (prop.type == Clor)
		{
			writer.Write(prop.propName.c_str(), *((Color*)prop.value));
		}
	}
}

void MetaData::Copy(void* source)
{
	for (int i = 0; i < properties.size(); i++)
	{
		Property& prop = properties[i];

		uint8_t* src = (uint8_t*)source + prop.offset;

		if (prop.type == Boolean)
		{
			memcpy(prop.value, src, sizeof(bool));
		}
		else
		if (prop.type == Integer || prop.type == Enum)
		{
			memcpy(prop.value, src, sizeof(int));
		}
		else
		if (prop.type == Float)
		{
			memcpy(prop.value, src, sizeof(float));
		}
		else
		if (prop.type == String || prop.type == FileName)
		{
			*((std::string*)prop.value) = *((std::string*)src);
		}
		else
		if (prop.type == Clor)
		{
			memcpy(prop.value, src, sizeof(float) * 4);
		}
	}
}

#ifdef EDITOR
void MetaData::PrepareWidgets(EUICategories* parent)
{
	for (auto& prop : properties)
	{
		if (!widgets_inited)
		{
			if (prop.type == Boolean)
			{
				prop.widget = new BoolWidget();
			}
			else
			if (prop.type == Integer)
			{
				prop.widget = new IntWidget();
			}
			else
			if (prop.type == Float)
			{
				prop.widget = new FloatWidget();
			}
			else
			if (prop.type == String)
			{
				prop.widget = new StringWidget();
			}
			else
			if (prop.type == FileName)
			{
				prop.widget = new FileNameWidget();
			}
			else
			if (prop.type == Clor)
			{
				prop.widget = new ColorWidget();
			}
			else
			if (prop.type == Enum)
			{
				prop.widget = new EnumWidget(&enums[prop.defvalue.enumIndex]);
			}
			else
			if (prop.type == Callback)
			{
				prop.widget = new CallbackWidget();
			}

			prop.widget->Init(parent, prop.catName.c_str(), prop.propName.c_str());
		}

		if (prop.type == Callback)
		{
			((CallbackWidget*)prop.widget)->Prepare(owner, prop.callback);
		}
		else
		{
			prop.widget->SetData(prop.value);
		}

		prop.widget->Show(true);
	}

	if (!widgets_inited)
	{
		widgets_inited = true;
	}
}

void MetaData::UpdateWidgets()
{
	for (auto& prop : properties)
	{
		if (prop.type == Callback)
		{
			((CallbackWidget*)prop.widget)->Prepare(owner, prop.callback);
		}
		else
		{
			prop.widget->SetData(prop.value);
		}
	}
}

bool MetaData::IsValueWasChanged()
{
	bool res = false;
	for (int i = 0; i < properties.size(); i++)
	{
		Property& prop = properties[i];
		res |= prop.widget->changed;
		prop.widget->changed = false;
	}

	return res;
}

void MetaData::HideWidgets()
{
	if (!widgets_inited)
	{
		return;
	}

	for (int i = 0; i < properties.size(); i++)
	{
		properties[i].widget->Show(false);
	}
}
#endif