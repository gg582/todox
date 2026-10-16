todox() {
    if [ "$1" = "-h" ] || [ "$1" = "--help" ] || [ "$1" = "help" ]; then
        command todox "$@"
        echo "bash extensions:"
        echo "  todox replace <title> <new_time(HH:MM:SS)>: replace selected title to a new time"
        return
    fi

    if [ "$1" = "replace" ]; then
        local target_title="$2"
        local new_time="$3"

        if [ -z "$target_title" ] || [ -z "$new_time" ]; then
            echo "  todox replace <title> <new_time(HH:MM:SS)>: replace selected title to a new time"
            return 1
        fi

        local raw_list
        raw_list="$(command todox)"

        local matched
        matched="$(printf '%s\n' "$raw_list" | awk -F '[ ]*\\|[ ]*' -v t="$target_title" '$2 == t')"

        if [ -z "$matched" ]; then
            echo "Error: Entry '$target_title' not found." >&2
            return 1
        fi

        # extract timezone, description, and repeat flag from the first matching row
        local first_line
        first_line="$(printf '%s\n' "$matched" | head -n 1)"

        local datetime_tz desc is_repeat tz
        datetime_tz="$(printf '%s\n' "$first_line" | awk -F '[ ]*\\|[ ]*' '{print $1}')"
        desc="$(printf '%s\n' "$first_line" | awk -F '[ ]*\\|[ ]*' '{print $3}')"
        is_repeat="$(printf '%s\n' "$first_line" | awk -F '[ ]*\\|[ ]*' '{print $4}')"
        tz="$(printf '%s\n' "$datetime_tz" | awk '{print $3}')"
        [ -z "$tz" ] && tz="$(date +%z)"

        local pattern
        if [ "$is_repeat" = "repeat" ]; then
            # extract weekdays from dates to reconstruct pattern
            local days
            days="$(printf '%s\n' "$matched" | awk -F '[ ]*\\|[ ]*' '{print $1}' | awk '{print $1}' | sort -u | while read -r d; do
                [ -n "$d" ] && LC_ALL=C date -d "$d" +%u 2>/dev/null
            done | sort -nu | tr '\n' ' ' | sed 's/[[:space:]]*$//')"

            case "$days" in
                "1 2 3 4 5")
                    pattern="mon-fri"
                    ;;
                "6 7")
                    pattern="sat-sun"
                    ;;
                "1 2 3 4 5 6 7")
                    pattern="mon-sun"
                    ;;
                *)
                    local map=("" "mon" "tue" "wed" "thu" "fri" "sat" "sun")
                    local parsed=()
                    for num in $days; do
                        parsed+=("${map[$num]}")
                    done
                    pattern="$(IFS=,; echo "${parsed[*]}")"
                    ;;
            esac
        else
            pattern="$(printf '%s\n' "$datetime_tz" | awk '{print $1}')"
        fi

        local payload="${pattern}%%${new_time} ${tz}%%${target_title}%%${desc}"

        command todox remove "$target_title" && command todox add "$payload"
    else
        command todox "$@"
    fi
}
