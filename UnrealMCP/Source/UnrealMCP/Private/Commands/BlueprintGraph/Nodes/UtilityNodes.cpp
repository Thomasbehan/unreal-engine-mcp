#include "Commands/BlueprintGraph/Nodes/UtilityNodes.h"
#include "Commands/BlueprintGraph/Nodes/NodeCreatorUtils.h"
#include "K2Node_CallFunction.h"
#include "K2Node_Select.h"
#include "K2Node_SpawnActorFromClass.h"
#include "EdGraphSchema_K2.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Engine/Blueprint.h"
#include "Json.h"

UK2Node* FUtilityNodeCreator::CreatePrintNode(UEdGraph* Graph, const TSharedPtr<FJsonObject>& Params)
{
	if (!Graph || !Params.IsValid())
	{
		return nullptr;
	}

	UK2Node_CallFunction* PrintNode = NewObject<UK2Node_CallFunction>(Graph);
	if (!PrintNode)
	{
		return nullptr;
	}

	UFunction* PrintFunc = UKismetSystemLibrary::StaticClass()->FindFunctionByName(
		GET_FUNCTION_NAME_CHECKED(UKismetSystemLibrary, PrintString)
	);

	if (!PrintFunc)
	{
		return nullptr;
	}

	PrintNode->SetFromFunction(PrintFunc);

	double PosX, PosY;
	FNodeCreatorUtils::ExtractNodePosition(Params, PosX, PosY);
	PrintNode->NodePosX = static_cast<int32>(PosX);
	PrintNode->NodePosY = static_cast<int32>(PosY);

	Graph->AddNode(PrintNode, true, false);
	FNodeCreatorUtils::InitializeK2Node(PrintNode, Graph);

	FString Message;
	if (Params->TryGetStringField(TEXT("message"), Message))
	{
		UEdGraphPin* InStringPin = PrintNode->FindPin(TEXT("InString"));
		if (InStringPin)
		{
			InStringPin->DefaultValue = Message;
		}
	}

	return PrintNode;
}

UK2Node* FUtilityNodeCreator::CreateCallFunctionNode(UEdGraph* Graph, const TSharedPtr<FJsonObject>& Params)
{
	if (!Graph || !Params.IsValid())
	{
		return nullptr;
	}

	FString TargetFunction;
	if (!Params->TryGetStringField(TEXT("target_function"), TargetFunction))
	{
		UE_LOG(LogTemp, Error, TEXT("CreateCallFunctionNode: Missing target_function parameter"));
		return nullptr;
	}

	UK2Node_CallFunction* CallNode = NewObject<UK2Node_CallFunction>(Graph);
	if (!CallNode)
	{
		return nullptr;
	}

	bool bCallSelf = false;
	Params->TryGetBoolField(TEXT("call_self"), bCallSelf);

	UFunction* TargetFunc = nullptr;

	if (bCallSelf)
	{
		UBlueprint* Blueprint = FBlueprintEditorUtils::FindBlueprintForGraph(Graph);
		if (Blueprint)
		{
			if (Blueprint->SkeletonGeneratedClass)
			{
				TargetFunc = Blueprint->SkeletonGeneratedClass->FindFunctionByName(FName(*TargetFunction));
			}
			if (!TargetFunc && Blueprint->GeneratedClass)
			{
				TargetFunc = Blueprint->GeneratedClass->FindFunctionByName(FName(*TargetFunction));
			}
			CallNode->FunctionReference.SetSelfMember(FName(*TargetFunction));
			UE_LOG(LogTemp, Display, TEXT("CreateCallFunctionNode: Set self-member function: %s"), *TargetFunction);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("CreateCallFunctionNode: Could not find Blueprint for self function call"));
			return nullptr;
		}
	}
	else
	{
		FString ClassName;
		if (Params->TryGetStringField(TEXT("target_class"), ClassName))
		{
			UClass* TargetClass = FindObject<UClass>(nullptr, *ClassName);
			if (!TargetClass)
			{
				TargetClass = LoadObject<UClass>(nullptr, *ClassName);
			}
			if (TargetClass)
			{
				TargetFunc = TargetClass->FindFunctionByName(FName(*TargetFunction));
			}
		}

		if (!TargetFunc)
		{
			TargetFunc = UKismetSystemLibrary::StaticClass()->FindFunctionByName(FName(*TargetFunction));
		}
		if (!TargetFunc)
		{
			TargetFunc = UKismetMathLibrary::StaticClass()->FindFunctionByName(FName(*TargetFunction));
		}

		if (!TargetFunc)
		{
			UE_LOG(LogTemp, Error, TEXT("CreateCallFunctionNode: Function not found: %s"), *TargetFunction);
			return nullptr;
		}

		CallNode->SetFromFunction(TargetFunc);
	}

	double PosX, PosY;
	FNodeCreatorUtils::ExtractNodePosition(Params, PosX, PosY);
	CallNode->NodePosX = static_cast<int32>(PosX);
	CallNode->NodePosY = static_cast<int32>(PosY);

	Graph->AddNode(CallNode, true, false);
	FNodeCreatorUtils::InitializeK2Node(CallNode, Graph);

	return CallNode;
}

FName FUtilityNodeCreator::GetMathFunctionName(const FString& Operation)
{
	static TMap<FString, FName> OperationMap = {
		{TEXT("Add"), TEXT("Add_DoubleDouble")},
		{TEXT("Subtract"), TEXT("Subtract_DoubleDouble")},
		{TEXT("Multiply"), TEXT("Multiply_DoubleDouble")},
		{TEXT("Divide"), TEXT("Divide_DoubleDouble")},
		{TEXT("Add_DoubleDouble"), TEXT("Add_DoubleDouble")},
		{TEXT("Subtract_DoubleDouble"), TEXT("Subtract_DoubleDouble")},
		{TEXT("Multiply_DoubleDouble"), TEXT("Multiply_DoubleDouble")},
		{TEXT("Divide_DoubleDouble"), TEXT("Divide_DoubleDouble")},
		{TEXT("Add_IntInt"), TEXT("Add_IntInt")},
		{TEXT("Subtract_IntInt"), TEXT("Subtract_IntInt")},
		{TEXT("Multiply_IntInt"), TEXT("Multiply_IntInt")},
		{TEXT("Sin"), TEXT("Sin")},
		{TEXT("Cos"), TEXT("Cos")},
		{TEXT("Tan"), TEXT("Tan")},
		{TEXT("Abs"), TEXT("Abs")},
		{TEXT("Sqrt"), TEXT("Sqrt")},
		{TEXT("Power"), TEXT("Power")},
		{TEXT("Add_VectorVector"), TEXT("Add_VectorVector")},
		{TEXT("Subtract_VectorVector"), TEXT("Subtract_VectorVector")},
		{TEXT("Multiply_VectorFloat"), TEXT("Multiply_VectorFloat")},
		{TEXT("Multiply_VectorVector"), TEXT("Multiply_VectorVector")},
		{TEXT("VectorLength"), TEXT("VSize")},
		{TEXT("Normalize"), TEXT("Normal")},
		{TEXT("DotProduct"), TEXT("Dot_VectorVector")},
		{TEXT("CrossProduct"), TEXT("Cross_VectorVector")},
		{TEXT("EqualEqual_FloatFloat"), TEXT("EqualEqual_FloatFloat")},
		{TEXT("NotEqual_FloatFloat"), TEXT("NotEqual_FloatFloat")},
		{TEXT("Less_FloatFloat"), TEXT("Less_FloatFloat")},
		{TEXT("Greater_FloatFloat"), TEXT("Greater_FloatFloat")},
		{TEXT("LessEqual_FloatFloat"), TEXT("LessEqual_FloatFloat")},
		{TEXT("GreaterEqual_FloatFloat"), TEXT("GreaterEqual_FloatFloat")},
		{TEXT("EqualEqual_IntInt"), TEXT("EqualEqual_IntInt")},
		{TEXT("Less_IntInt"), TEXT("Less_IntInt")},
		{TEXT("Greater_IntInt"), TEXT("Greater_IntInt")},
		{TEXT("Conv_IntToFloat"), TEXT("Conv_IntToFloat")},
		{TEXT("Round"), TEXT("Round")},
		{TEXT("Floor"), TEXT("FFloor")},
		{TEXT("Lerp"), TEXT("Lerp")},
		{TEXT("FClamp"), TEXT("FClamp")},
		{TEXT("Clamp"), TEXT("Clamp")},
		{TEXT("FMin"), TEXT("FMin")},
		{TEXT("FMax"), TEXT("FMax")},
		{TEXT("RandomFloatInRange"), TEXT("RandomFloatInRange")},
		{TEXT("RandomIntegerInRange"), TEXT("RandomIntegerInRange")},
		{TEXT("FInterpTo"), TEXT("FInterpTo")},
		{TEXT("VInterpTo"), TEXT("VInterpTo")},
		{TEXT("MakeRotator"), TEXT("MakeRotator")},
	};

	const FName* FoundName = OperationMap.Find(Operation);
	if (FoundName)
	{
		return *FoundName;
	}
	return FName(*Operation);
}

UK2Node* FUtilityNodeCreator::CreateMathNode(UEdGraph* Graph, const TSharedPtr<FJsonObject>& Params)
{
	if (!Graph || !Params.IsValid())
	{
		return nullptr;
	}

	FString Operation;
	if (!Params->TryGetStringField(TEXT("operation"), Operation) && !Params->TryGetStringField(TEXT("target_function"), Operation))
	{
		UE_LOG(LogTemp, Error, TEXT("CreateMathNode: Missing operation parameter"));
		return nullptr;
	}

	FName FunctionName = GetMathFunctionName(Operation);
	UFunction* MathFunc = UKismetMathLibrary::StaticClass()->FindFunctionByName(FunctionName);
	if (!MathFunc)
	{
		UE_LOG(LogTemp, Error, TEXT("CreateMathNode: Math function not found: %s"), *FunctionName.ToString());
		return nullptr;
	}

	UK2Node_CallFunction* MathNode = NewObject<UK2Node_CallFunction>(Graph);
	if (!MathNode)
	{
		return nullptr;
	}

	MathNode->SetFromFunction(MathFunc);

	double PosX, PosY;
	FNodeCreatorUtils::ExtractNodePosition(Params, PosX, PosY);
	MathNode->NodePosX = static_cast<int32>(PosX);
	MathNode->NodePosY = static_cast<int32>(PosY);

	Graph->AddNode(MathNode, true, false);
	FNodeCreatorUtils::InitializeK2Node(MathNode, Graph);

	UE_LOG(LogTemp, Display, TEXT("CreateMathNode: Created %s node"), *Operation);
	return MathNode;
}

UK2Node* FUtilityNodeCreator::CreateSelectNode(UEdGraph* Graph, const TSharedPtr<FJsonObject>& Params)
{
	if (!Graph || !Params.IsValid())
	{
		return nullptr;
	}

	UK2Node_Select* SelectNode = NewObject<UK2Node_Select>(Graph);
	if (!SelectNode)
	{
		return nullptr;
	}

	double PosX, PosY;
	FNodeCreatorUtils::ExtractNodePosition(Params, PosX, PosY);
	SelectNode->NodePosX = static_cast<int32>(PosX);
	SelectNode->NodePosY = static_cast<int32>(PosY);

	Graph->AddNode(SelectNode, true, false);
	FNodeCreatorUtils::InitializeK2Node(SelectNode, Graph);

	return SelectNode;
}

UK2Node* FUtilityNodeCreator::CreateSpawnActorNode(UEdGraph* Graph, const TSharedPtr<FJsonObject>& Params)
{
	if (!Graph || !Params.IsValid())
	{
		return nullptr;
	}

	UK2Node_SpawnActorFromClass* SpawnActorNode = NewObject<UK2Node_SpawnActorFromClass>(Graph);
	if (!SpawnActorNode)
	{
		return nullptr;
	}

	double PosX, PosY;
	FNodeCreatorUtils::ExtractNodePosition(Params, PosX, PosY);
	SpawnActorNode->NodePosX = static_cast<int32>(PosX);
	SpawnActorNode->NodePosY = static_cast<int32>(PosY);

	Graph->AddNode(SpawnActorNode, true, false);
	FNodeCreatorUtils::InitializeK2Node(SpawnActorNode, Graph);

	return SpawnActorNode;
}
