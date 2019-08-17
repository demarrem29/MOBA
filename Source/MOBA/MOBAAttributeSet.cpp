// Fill out your copyright notice in the Description page of Project Settings.


#include "MOBAAttributeSet.h"

UMOBAAttributeSet::UMOBAAttributeSet() 
	:Health(500.0f)
	,MaxHealth(500.0f)
	,Mana(300.0f)
	,MaxMana(300.0f)
	,Level(1.0f)
	,MaxLevel(18.0f)
	,Experience(0.0f)
	,MaxExperience(280.0f)
{
	static ConstructorHelpers::FObjectFinder<UDataTable>
		ExperiencePerLevelObject(TEXT("DataTable'/Game/Data/ExperiencePerLevel.ExperiencePerLevel'"));
	if (ExperiencePerLevelObject.Succeeded())
	{
		ExperiencePerLevelData = ExperiencePerLevelObject.Object;
	}
}



void UMOBAAttributeSet::PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data)
{
	UAbilitySystemComponent* Source = Data.EffectSpec.GetContext().GetOriginalInstigatorAbilitySystemComponent();

	if (HealthAttribute() == Data.EvaluatedData.Attribute)
	{
		// Get the Target actor
		AActor* DamagedActor = nullptr;
		AController* DamagedController = nullptr;
		if (Data.Target.AbilityActorInfo.IsValid() && Data.Target.AbilityActorInfo->AvatarActor.IsValid())
		{
			DamagedActor = Data.Target.AbilityActorInfo->AvatarActor.Get();
			DamagedController = Data.Target.AbilityActorInfo->PlayerController.Get();
		}

		// Get the Source actor
		AActor* AttackingActor = nullptr;
		AController* AttackingController = nullptr;
		AController* AttackingPlayerController = nullptr;
		if (Source && Source->AbilityActorInfo.IsValid() && Source->AbilityActorInfo->AvatarActor.IsValid())
		{
			AttackingActor = Source->AbilityActorInfo->AvatarActor.Get();
			AttackingController = Source->AbilityActorInfo->PlayerController.Get();
			AttackingPlayerController = Source->AbilityActorInfo->PlayerController.Get();
			if (AttackingController == nullptr && AttackingActor != nullptr)
			{
				if (APawn * Pawn = Cast<APawn>(AttackingActor))
				{
					AttackingController = Pawn->GetController();
				}
			}
		}

		// Clamp health
		Health = FMath::Clamp(Health.GetCurrentValue(), 0.0f, MaxHealth.GetCurrentValue());
		if (Health.GetCurrentValue() <= 0)
		{
			/*
			// Handle death with GASCharacter. Note this is just one example of how this could be done.
			if (AGASCharacter * GASChar = Cast<AGASCharacter>(DamagedActor))
			{
				// Construct a gameplay cue event for this death
				FGameplayCueParameters Params(Data.EffectSpec.GetContext());
				Params.RawMagnitude = Data.EvaluatedData.Magnitude;;
				Params.NormalizedMagnitude = FMath::Abs(Data.EvaluatedData.Magnitude / MaxHealth);
				Params.AggregatedSourceTags = *Data.EffectSpec.CapturedSourceTags.GetAggregatedTags();
				Params.AggregatedTargetTags = *Data.EffectSpec.CapturedTargetTags.GetAggregatedTags();

				GASChar->Die(DamagedController, DamagedActor, Data.EffectSpec, Params.RawMagnitude, Params.Normal);
			}*/
			HealthChange.Broadcast(Health,MaxHealth);
		}
	}
	if (ManaAttribute() == Data.EvaluatedData.Attribute)
	{
		Mana = FMath::Clamp(Mana.GetCurrentValue(), 0.0f, MaxMana.GetCurrentValue());
		ManaChange.Broadcast(Mana, MaxMana);
	}
	if (LevelAttribute() == Data.EvaluatedData.Attribute) 
	{
		Level = FMath::Clamp(Level.GetCurrentValue(), 1.0f, MaxLevel.GetCurrentValue());
		if (Level.GetCurrentValue() == MaxLevel.GetCurrentValue()) 
		{
			Experience = 0;
			MaxExperience = 0;
		}
		LevelChange.Broadcast(Level, MaxLevel);
	}
	if (ExperienceAttribute() == Data.EvaluatedData.Attribute)
	{
		if (Level.GetCurrentValue() < MaxLevel.GetCurrentValue()) {		// Only check XP if level is less than max level
			FString currentlevelstring = FString::FromInt(static_cast<int32>(Level.GetCurrentValue()));	// Get current level as a string
			FExperiencePerLevel* XPToLvl = ExperiencePerLevelData->FindRow<FExperiencePerLevel>(FName(*currentlevelstring), "Experience Per Level", true); // Grab the row for the current level
			if (XPToLvl)
			{
				if (Experience.GetCurrentValue() >= XPToLvl->Experience) // Check if we have enough experience for a level up
				{
					Experience = 0;
					Level = (Level.GetCurrentValue() + 1);
					LevelChange.Broadcast(Level, MaxLevel);
					currentlevelstring = FString::FromInt(static_cast<int32>(Level.GetCurrentValue()));	// Get current level as a string
					XPToLvl = ExperiencePerLevelData->FindRow<FExperiencePerLevel>(FName(*currentlevelstring), "Experience Per Level", true); // Grab the row for the current level
					MaxExperience = XPToLvl->Experience;
				}
			}
		}
		ExperienceChange.Broadcast(Experience, MaxExperience);
	}
}

FGameplayAttribute UMOBAAttributeSet::HealthAttribute()
{
	static UProperty* Property = FindFieldChecked<UProperty>(UMOBAAttributeSet::StaticClass(), GET_MEMBER_NAME_CHECKED(UMOBAAttributeSet, Health));
	return FGameplayAttribute(Property);
}

FGameplayAttribute UMOBAAttributeSet::ManaAttribute()
{
	static UProperty* Property = FindFieldChecked<UProperty>(UMOBAAttributeSet::StaticClass(), GET_MEMBER_NAME_CHECKED(UMOBAAttributeSet, Mana));
	return FGameplayAttribute(Property);
}

FGameplayAttribute UMOBAAttributeSet::LevelAttribute()
{
	static UProperty* Property = FindFieldChecked<UProperty>(UMOBAAttributeSet::StaticClass(), GET_MEMBER_NAME_CHECKED(UMOBAAttributeSet, Level));
	return FGameplayAttribute(Property);
}

FGameplayAttribute UMOBAAttributeSet::ExperienceAttribute()
{
	static UProperty* Property = FindFieldChecked<UProperty>(UMOBAAttributeSet::StaticClass(), GET_MEMBER_NAME_CHECKED(UMOBAAttributeSet, Experience));
	return FGameplayAttribute(Property);
}
