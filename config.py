# config.py


def can_build(env, platform):
    return True


def configure(env):
    pass


def get_doc_path():
    return "doc_classes"


def get_doc_classes():
    return [
        "BBAabb",
        "BBArray",
        "BBBasis",
        "BBBool",
        "BBByteArray",
        "BBColor",
        "BBColorArray",
        "BBDictionary",
        "BBFloat",
        "BBInt",
        "BBIntArray",
        "BBNode",
        "BBParam",
        "BBPlane",
        "BBQuat",
        "BBRealArray",
        "BBRect2",
        "BBString",
        "BBStringArray",
        "BBTransform",
        "BBTransform2D",
        "BBVariant",
        "BBVector2",
        "BBVector2Array",
        "BBVector3",
        "BBVector3Array",
        "BehaviorTree",
        "Blackboard",
        "BTAction",
        "BTAlwaysFail",
        "BTAlwaysSucceed",
        "BTComposite",
        "BTCondition",
        "BTConsolePrint",
        "BTCooldown",
        "BTDecorator",
        "BTDelay",
        "BTDynamicSelector",
        "BTDynamicSequence",
        "BTFail",
        "BTForEach",
        "BTInvert",
        "BTNewScope",
        "BTParallel",
        "BTPlayer",
        "BTProbability",
        "BTRandomSelector",
        "BTRandomSequence",
        "BTRandomWait",
        "BTRepeat",
        "BTRepeatUntilFailure",
        "BTRepeatUntilSuccess",
        "BTRunLimit",
        "BTSelector",
        "BTSequence",
        "BTState",
        "BTSubtree",
        "BTTask",
        "BTTimeLimit",
        "BTWait",
        "BTWaitTicks",
        "LimboHSM",
        "LimboState",
        "LimboUtility",
    ]
