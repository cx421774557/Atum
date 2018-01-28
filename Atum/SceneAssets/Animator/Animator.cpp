
#include "Animator.h"

CLASSDECLDECL(SceneAsset, Animator)

META_DATA_DESC(Animator)
FLOAT_PROP(Animator, timeLenght, 3.0f, "Prop", "TimeLenght")
META_DATA_DESC_END()

void Animator::Init()
{
#ifdef EDITOR
	wnd = nullptr;
#endif

	edCurTime = 0.0f;
	fromTime = 0.0f;
	toTime = 3.0f;
}

void Animator::Load(JSONReader* stream)
{
	GetMetaData()->Prepare(this);
	GetMetaData()->Load(stream);
	toTime = timeLenght;

	int num = 0;
	stream->Read("NumPlayers", num);
	players.resize(num);

	for (int i=0;i<num;i++)
	{
		if (!stream->EnterBlock("TrackPlayer")) break;

		char tp[64];
		stream->Read("Type", tp, 64);

		players[i] = ClassFactoryTrackPlayer::Create(tp);
		players[i]->owner = this;

		if (players[i])
		{
			players[i]->SetName(tp);
			players[i]->SetType(tp);

			players[i]->Init();

			players[i]->Load(stream);
		}

		stream->LeaveBlock();
	}
}

void Animator::Save(JSONWriter* stream)
{
	GetMetaData()->Prepare(this);
	GetMetaData()->Save(stream);
	
	stream->Write("NumPlayers", (int)players.size());

	stream->StartArray("TrackPlayer");

	for (int i=0;i<players.size();i++)
	{
		stream->StartBlock(nullptr);

		players[i]->Save(stream);

		stream->FinishBlock();
	}

	stream->FinishArray();
}

void Animator::SetEditMode(bool ed)
{
	//SceneEntity::SetEditMode(ed);

#ifdef EDITOR
	if (!wnd)
	{
		if (!AnimatorWindow::instance)
		{
			AnimatorWindow::instance = new AnimatorWindow();
			AnimatorWindow::instance->Init();
		}

		wnd = AnimatorWindow::instance;
	}

	wnd->PrepareWidgets(this);
	wnd->Show(ed);

	if (ed)
	{
		Tasks()->AddTask(0, this, (Object::Delegate)&Animator::Work);
	}
	else
	{
		Tasks()->DelTask(0, this);
	}
#endif
}

void Animator::Work(float dt)
{
	if (wnd->selTrack != -1)
	{
		players[wnd->selPlayer]->tracks[wnd->selTrack]->DebugDraw();

		if (wnd->selTrack != -1)
		{
			players[wnd->selPlayer]->tracks[wnd->selTrack]->DataFromGizmo();
		}
	}

	for (int i = 0; i<players.size(); i++)
	{
		if (!players[i]->IsActive()) continue;

		players[i]->Reset();
		players[i]->SetTime(edCurTime);
	}
}

void Animator::Reset()
{
	for (int i=0;i<players.size();i++)
	{
		players[i]->Reset();
	}
}