def get_param_name(param_id: str) -> str:
    """Get the parameter name from the ID."""
    #  remove PARAM_ prefix and convert to lowercase
    return param_id.replace("PARAM_", "").lower().replace("_", " ").capitalize()
