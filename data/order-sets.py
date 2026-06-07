def sort_file(file):
    with open(file, "r") as f:
        words = [line.strip() for line in f if line.strip()]
        words.sort()
        with open(file, "w") as f:
            for word in words:
                f.write(f"{word}\n")


if __name__ == "__main__":
    sort_file("guess-set.txt")
    with open("candidate-set.txt", "r") as f:
        candidate_words = [line.strip() for line in f if line.strip()]
        word_indices = []
        with open("guess-set.txt", "r") as t:
            guess_words = [line.strip() for line in t if line.strip()]
            for word in candidate_words:
                word_indices.append(guess_words.index(word))

        with open("candidate-set-indices.txt", "w") as f:
            for idx in word_indices:
                f.write(f"{idx}\n")
